/**
 * dns-tunneler
 *
 * @file connections.c
 *
 * @brief Connections handling module.
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "connections.h"
#include "dns_receiver_events.h"
#include "../common/dns.h"
#include "../common/base64.h"
#include "../common/communication.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int tcp_socket = -1;
int udp_socket = -1;

int start_tcp(struct sockaddr_in6 *address)
{
  if ((tcp_socket = socket_factory(address, SOCK_STREAM, 1)) == -1)
    return -1;

  if (listen(tcp_socket, MAX_CONN) < 0)
    ERROR_EXIT("listen()", -1);

  return tcp_socket;
}

/**
 * Function starts UDP dual stack server.
 * @param address address struct to listen on
 * @return fd of new socket; -1 on error
 */
int start_udp(struct sockaddr_in6 *address)
{
  if ((udp_socket = socket_factory(address, SOCK_DGRAM, 1)) == -1)
    return -1;

  return udp_socket;
}

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 *
 * Taken from https://github.com/lattera/freebsd/blob/master/lib/libc/string/strnstr.c
 * BSD license.
 */
char *
strnstr(const char *s, const char *find, size_t slen)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != '\0') {
    len = strlen(find);
    do {
      do {
        if (slen-- < 1 || (sc = *s++) == '\0')
          return (NULL);
      } while (sc != c);
      if (len > slen)
        return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}

/**
 * Starts both TCP and UDP server running on same port and address.
 * Afterwards prepares variable `max_socket` with bigger fd.
 * Function sets global variables `tcp_socket` and `udp_socket` to
 * corresponding FD. Caller is responsible of closing socket after
 * usage.
 *
 * @param address address structure to listen on
 * @return returns FD of max socket; -1 on error (on error function closes remaining opened FDs)
 */
int start_both(struct sockaddr_in6 *address)
{
  PREPARE_ADDRESS_PTR(address, in6addr_any, DNS_PORT);

  tcp_socket = start_tcp(address);
  udp_socket = start_udp(address);

  int max_socket = tcp_socket;
  int min_socket = udp_socket;
  // ^^^ used to -1 (a.k.a error during socket init) detection

  if (udp_socket > tcp_socket)
  {
      max_socket = udp_socket;
      min_socket = tcp_socket;
  }

  if (min_socket == -1)
  {
      close(max_socket);
      return -1;
  }

  return max_socket;
}

/**
 * This is some kind of "proxy" it just sets truncated flag to 1 and
 * forwards packet back to the sender as a acknowledge it has been delivered
 * and receiver is waiting for next chunk.
 *
 * @param sock sock to be used
 * @param hdr header of DNS packet
 * @param data data
 * @param q question structure
 * @return length of written data to the socket
 */
int send_ack(int sock, header *hdr, char *data, question *q)
{
  hdr->tc = 1;
  hdr->qr = 1;
  int len = write(sock, hdr, sizeof(header));
  len += write(sock, data, strlen(data) + 1);
  len += write(sock, q, sizeof(question));

  return len;
}

/**
 * Function is BLOCKING, it waits for first packet with file name.
 * Then it tries to open file if opening fails, NULL is returned.
 *
 * @param cfg receiver config
 * @param sock sock to read data from
 * @return fd of file to write in; NULL on error
 */
FILE *output(receiver_config *cfg, int sock)
{
  printf("waiting for filename packet\n");
  char buffer[PACKET_BUFFER_SIZE + 1] = {0};
  size_t len = 0;
  header hdr = {0};
  question q = {0};

  char domain[MAX_QUERY_LEN];
  convert_to_dns_format(domain, cfg->sneaky_domain);

  while (!strnstr(&(buffer[sizeof(header)]), domain, len))  // receiving until domain name
    len += recv(sock, &(buffer[len]), PACKET_BUFFER_SIZE - len, MSG_DONTWAIT);

  char *file = retype_parts(buffer, &hdr, &q);

  while(recv(sock, buffer, PACKET_BUFFER_SIZE, MSG_DONTWAIT) > 0)  // receive the rest of query
      ;

  send_ack(sock, &hdr, file, &q);

  remove_domain(file, cfg->sneaky_domain);
  printf("got filename packet: %s\n", file);

  FILE *f = fopen(++file, "wb+");  // ++ to move ptr behind label len

  return f;
}

#define CLOSE_FDS_RET(f, ret) do{fclose(f); return ret;}while(0)
/**
 * Function downloads file from socket sent via DNS query packets.
 * It also handles first initialization packet with file name.
 *
 * @param cfg receiver config
 * @param sock socket to download data from
 * @return siz of downloaded data; -1 on error
 */
int download_file(receiver_config *cfg, int sock)
{
  char buffer[PACKET_BUFFER_SIZE + 1];
  header hdr = {0};
  question q = {0};
  int total = 0, decoded_len;

  FILE *f = output(cfg, sock);
  if (!f)
    ERROR_EXIT("Could not open destination file\n", -1);

  while (1) {
    if (receive_dns_packet(sock, buffer) == -1)
        break;  // file has been downloaded

    char *data = retype_parts(buffer, &hdr, &q);
    send_ack(sock, &hdr, data, &q);

    remove_domain(data, cfg->sneaky_domain);
    decoded_len = dechunkize(data, strlen(data));

    char *decoded = base64_decode(data, strlen(data), &decoded_len);
    if (!decoded || decoded_len <= 0)
        CLOSE_FDS_RET(f, -1);

    fwrite(decoded, 1, decoded_len, f);
  }
  printf("DONE\n");
  CLOSE_FDS_RET(f, total);
}
#undef CLOSE_FDS

/**
 * Function processes TCP packet
 *
 * @param cfg receiver config
 * @param client structure of client
 * @param addrlen length of address
 */
void process_tcp_query(receiver_config *cfg, struct sockaddr_in6 *client, int *addrlen)
{
  printf("TCP\n");
  int connection;
  if ((connection = accept(tcp_socket, (struct sockaddr *)&client,
                           (socklen_t *)&addrlen)) < 0)
  {
    fprintf(stderr, "Could not accept connection\n");
    return;
  }
  printf("accepted\n");
  download_file(cfg, connection);
  //close(connection);
}

/**
 * Function sends UDP reply with truncated flag set to 1 which means
 * client should switch to TCP connection.
 *
 * @param client client address
 * @param domain sneaky domain
 * @param hdr header structure from DNS query
 */
void request_protocol_switch(struct sockaddr_in6 *client, char *domain, header *hdr)
{
  char packet[PACKET_BUFFER_SIZE] = {0};
  int response_len;

  prepare_packet(packet, &response_len, domain, hdr->id, 1, 1, 1);
  sendto(udp_socket, packet, response_len, 0, (struct sockaddr *)client, sizeof(*client));
}

/**
 * Function processes UDP packet. It acts as DNS proxy if domain
 * from request does not correspond to sneaky domain.
 *
 * @param client client address
 * @param addrlen length of client address
 * @param sneaky_domain domain that is should be used for file transfer
 */
void process_udp_query(struct sockaddr_in6 *client, int *addrlen, char *sneaky_domain)
{
  printf("UDP\n");

  char buffer[PACKET_BUFFER_SIZE] = {0};
  size_t received = recvfrom(udp_socket, buffer, PACKET_BUFFER_SIZE, 0, ( struct sockaddr *) client, addrlen);
  if (received <= (sizeof(header) + 4))  // 4 as minimal domain len: "a.sk\0"
  {
    fprintf(stderr, "Packet is smaller than sizeof(head) + minimal domain len\n");
    return;
  }

  header hdr;
  question q;

  char *domain = retype_parts(buffer, &hdr, &q);
  printf("id: %04x; type: %d: %s\n", ntohs(hdr.id), htons(q.qtype), domain);

  if (is_transfer_request(domain, sneaky_domain))
    request_protocol_switch(client, domain, &hdr);
  else  // dns proxy
    resolve(buffer, received, client, udp_socket);
}

/**
 * Function checks for incoming packets both on TCP and UDP sockets
 * using select(7) function whether there is something to handle.
 *
 * @param cfg receiver config
 * @return 1 on error; otherwise should run endlessly
 */
int listen_for_queries(receiver_config *cfg)
{
  struct sockaddr_in6 address, client;
  int addrlen = sizeof(client), max_socket, ret;
  max_socket = start_both(&address);
  if (max_socket == -1)
      return 1;

  printf("Listening on %d (both TCP and UDP)\n", DNS_PORT);

  while (1) // endless loop for accepting connections endlessly
  {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(tcp_socket, &fds);
    FD_SET(udp_socket, &fds);

    struct timeval timeouts = {
        .tv_usec = 50
    };

    if ((ret = select(max_socket + 1, &fds, NULL, NULL, &timeouts)) == SOCKET_ERROR)
      ERROR_EXIT("Could not select socket!\n", EXIT_FAILURE);
    else if (ret == 0)
      continue;  // timeout passed

    if (FD_ISSET(tcp_socket, &fds)) {
        process_tcp_query(cfg, &client, &addrlen);
        printf("donw\n");
    }
    else if(FD_ISSET(udp_socket, &fds))
      process_udp_query(&client, &addrlen, cfg->sneaky_domain);
  }
  return EXIT_SUCCESS;
}
