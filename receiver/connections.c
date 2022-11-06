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

#include "read_data.h"
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
#include <ctype.h>


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

char *get_filename(receiver_config *cfg, char *file)
{
  size_t path_len = strlen(cfg->dest_filepath);
  size_t file_len = strlen(file);
  size_t real_path_len = path_len + file_len + 2;
  char *path = (char *)calloc(real_path_len, sizeof(char));  // 1 for \0 and another for /

  if (!path)
    return NULL;

  strcpy(path, cfg->dest_filepath);
  if (path[path_len] != '/')
    strcpy(&(path[path_len++]), "/");

  for (size_t i = path_len, j = 0; i < real_path_len && *(++file) != '\0'; i++)
    path[path_len + (j++)] = (isalnum(*file))? *file : '/';

  path[path_len + file_len] = '\0';

  return path;
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
  char buffer[PACKET_BUFFER_SIZE + 1] = {0};
  size_t len = 0;
  header hdr = {0};
  question q = {0};

  char domain[MAX_QUERY_LEN];
  convert_to_dns_format(domain, cfg->sneaky_domain);

  while(recv(sock, &(buffer[len]), PACKET_BUFFER_SIZE - len, 0) > 0)
    ;

  char *file = retype_parts(buffer, &hdr, &q);

  send_ack(sock, &hdr, file, &q);

  remove_domain(file, cfg->sneaky_domain);

  char *filename = get_filename(cfg, file);
  if (!filename)
  {
    printf("Invalid filename\n");
    return NULL;
  }
  cfg->real_path = filename;

  FILE *f = fopen(filename, "wb+");
  if (!f)
    close(sock);

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
int download_file(receiver_config *cfg, int sock, struct sockaddr_in6 *client)
{
  char buffer[PACKET_BUFFER_SIZE + 1];
  header hdr = {0};
  question q = {0};
  int total = 0, decoded_len, v6 = 1;

  FILE *f = output(cfg, sock);
  if (!f)
    ERROR_EXIT("Could not open destination file\n", -1);

  struct in_addr *raw_v4;
  if (IN6_IS_ADDR_V4MAPPED(&(client->sin6_addr))){
    v6 = 0;
    raw_v4 = &(client->sin6_addr.s6_addr[12]);
  }

  while (1) {
    if (receive_dns_packet(sock, buffer) == -1)
        break;  // file has been downloaded

    char *data = retype_parts(buffer, &hdr, &q);
    dns_receiver__on_query_parsed(cfg->real_path, data);
    send_ack(sock, &hdr, data, &q);

    remove_domain(data, cfg->sneaky_domain);
    decoded_len = dechunkize(data, strlen(data));

    if (v6)
      dns_receiver__on_chunk_received6(&(client->sin6_addr), cfg->real_path, hdr.id, decoded_len);
    else
      dns_receiver__on_chunk_received(raw_v4, cfg->real_path, hdr.id, decoded_len);

    char *decoded = base64_decode(data, strlen(data), &decoded_len);
    if (!decoded || decoded_len <= 0)
        CLOSE_FDS_RET(f, -1);
    total += decoded_len;

    fwrite(decoded, 1, decoded_len, f);
    free(decoded);
  }
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
  int connection;
  if ((connection = accept(tcp_socket, (struct sockaddr *)client,
                           (socklen_t *)addrlen)) < 0)
  {
    fprintf(stderr, "Could not accept connection\n");
    return;
  }

  if (!IN6_IS_ADDR_V4MAPPED(&(client->sin6_addr)))
    dns_receiver__on_transfer_init6(&(client->sin6_addr));
  else{
    SOCKADDR6_TO_4(client->sin6_addr);
    dns_receiver__on_transfer_init(raw_v4);
  }

  int size = download_file(cfg, connection, client);
  close(connection);
  dns_receiver__on_transfer_completed(cfg->real_path, size);
  if (cfg->real_path)
    free(cfg->real_path);
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
  memset(&address, 0, sizeof(struct sockaddr_in6));
  memset(&client, 0, sizeof(struct sockaddr_in6));
  struct in6_addr raw_addr = IN6ADDR_ANY_INIT;

  address.sin6_family = AF_INET6;
  address.sin6_addr = raw_addr;
  address.sin6_port = htons(DNS_PORT);

  int addrlen = sizeof(client), max_socket, ret;
  max_socket = start_both(&address);
  if (max_socket == -1)
      return 1;

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

    if (FD_ISSET(tcp_socket, &fds))
        process_tcp_query(cfg, &client, &addrlen);
    else if(FD_ISSET(udp_socket, &fds))
      process_udp_query(&client, &addrlen, cfg->sneaky_domain);
  }
  return EXIT_SUCCESS;
}
