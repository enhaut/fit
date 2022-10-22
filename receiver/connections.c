/**
 * dns-tunneler
 *
 * @file connections.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "connections.h"
#include "../common/dns.h"
#include "../common/base64.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int tcp_socket = -1;
int udp_socket = -1;

#define ERROR_EXIT(msg, ret) do{printf(msg); return ret;}while(0)




void prepare_address(struct sockaddr_in6 *address)
{
  address->sin6_family = AF_INET6;
  address->sin6_addr = in6addr_any;
  address->sin6_port = htons(DNS_PORT);
}

int socket_factory(struct sockaddr_in6 *address, int type)
{
  int generic_socket, t = 1, f = 0;

  if ((generic_socket = socket(AF_INET6, type, 0)) <= 0)
    ERROR_EXIT("socket()", -1);

  if (setsockopt(generic_socket, SOL_SOCKET, SO_REUSEADDR , &t, sizeof(t)))
    ERROR_EXIT("setsockopt()", -1);

  // Disable IPv6 only => accept both 4 and 6
  if (setsockopt(generic_socket, IPPROTO_IPV6, IPV6_V6ONLY, &f, sizeof(f)))
    ERROR_EXIT("ipv6()", -1);

  if (bind(generic_socket, (struct sockaddr *)address, sizeof(*address)) < 0)
    ERROR_EXIT("bind()", -1);

  return generic_socket;
}

int start_tcp(struct sockaddr_in6 *address)
{
  if ((tcp_socket = socket_factory(address, SOCK_STREAM)) == -1)
    return -1;

  if (listen(tcp_socket, MAX_CONN) < 0)
    ERROR_EXIT("listen()", -1);

  return tcp_socket;
}

int start_udp(struct sockaddr_in6 *address)
{
  if ((udp_socket = socket_factory(address, SOCK_DGRAM)) == -1)
    return -1;

  return udp_socket;
}

int start_both(struct sockaddr_in6 *address)
{
  prepare_address(address);

  tcp_socket = start_tcp(address);
  udp_socket = start_udp(address);

  int max_socket = tcp_socket;
  if (udp_socket > tcp_socket)
    max_socket = udp_socket;

  return max_socket;
}
int send_ack(int sock, header *hdr, char *data, question *q)
{
  hdr->tc = 1;
  hdr->qr = 1;
  int len = write(sock, hdr, sizeof(header));
  len += write(sock, data, strlen(data) + 1);
  len += write(sock, q, sizeof(question));

  return len;
}

FILE *output(receiver_config *cfg, int sock)
{
  printf("waiting for filename packet\n");
  char buffer[PACKET_BUFFER_SIZE + 1] = {0};
  size_t len = 0;
  header hdr = {0};
  question q = {0};

  char domain[MAX_QUERY_LEN];
  convert_to_dns_format(domain, cfg->sneaky_domain);

  while (!strnstr(&(buffer[sizeof(header)]), domain, len))  // TODO
    len += recv(sock, &(buffer[len]), PACKET_BUFFER_SIZE - len, MSG_DONTWAIT);

  char *file = retype_parts(buffer, &hdr, &q);

  while(recv(sock, buffer, PACKET_BUFFER_SIZE, MSG_DONTWAIT) > 0)
      ;

  send_ack(sock, &hdr, file, &q);

  remove_domain(file, cfg->sneaky_domain);
  printf("got filename packet: %s\n", file);

  FILE *f = fopen(++file, "wb+");  // ++ to move ptr behind label len

  return f;
}

int download_file(receiver_config *cfg, int sock)
{
  char buffer[PACKET_BUFFER_SIZE + 1];
  header hdr = {0};
  question q = {0};
  int total = 0, decoded_len;

  FILE *f = output(cfg, sock);
  if (!f)
      ERROR_EXIT("Could not open destination file\n", -1);

  while (1)
  {
    if (receive_dns_packet(sock, buffer) == -1)
        break;

    char *data = retype_parts(buffer, &hdr, &q);
    send_ack(sock, &hdr, data, &q);

    remove_domain(data++, cfg->sneaky_domain);
   // decoded_len = dechunkize(data, strlen(data));

    char *decoded = base64_decode(data, strlen(data), &decoded_len);
    if (!decoded || decoded_len <= 0) {
        printf("EROR\n");
        break;
    }

    fwrite(decoded, 1, decoded_len, f);
  }
  fclose(f);

  return total;
}

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
  close(connection);
}

void request_protocol_switch(struct sockaddr_in6 *client, char *domain, header *hdr)
{
  char packet[PACKET_BUFFER_SIZE] = {0};
  int response_len;

  prepare_packet(packet, &response_len, domain, hdr->id, 1, 1, 1);
  sendto(udp_socket, packet, response_len, 0, (struct sockaddr *)client, sizeof(*client));
}

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
  else
    resolve(buffer, received, client, udp_socket);
}

int listen_for_queries(receiver_config *cfg)
{
  struct sockaddr_in6 address, client;
  int addrlen = sizeof(client), max_socket, ret;
  max_socket = start_both(&address);

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

    if (FD_ISSET(tcp_socket, &fds))
      process_tcp_query(cfg, &client, &addrlen);
    else if(FD_ISSET(udp_socket, &fds))
      process_udp_query(&client, &addrlen, cfg->sneaky_domain);
  }
  return EXIT_SUCCESS;
}
