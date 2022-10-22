/**
 * dns-tunneler
 *
 * @file sender.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "sender.h"
#include "dns_sender_events.h"
#include "args_parser.h"
#include "../common/dns.h"
#include "../common/base64.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void wait_for_ack(int sock, int size)
{
  char packet[PACKET_BUFFER_SIZE + 1];

  printf("Waiting for ACK: %d\n", size);
  int len, total = 0;
  while (size > 0)
  {
    len = recv(sock, &(packet[total]), size, 0);
    if (len <= 0)
        continue;  // TODO: really?
    total += len;
    size -= len;
  }
  printf("got ack\n");
  // ^^ needed to block it in this way, because recv is non-blocking on my mac
}

int open_tcp_connection(sender_config *cfg)
{
  int sock, len;
  struct sockaddr_in servaddr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    ERROR_EXIT("Could not create TCP socket to server\n", 0);
  else
    printf("Socket successfully created..\n");

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(cfg->ip);
  servaddr.sin_port = htons(DNS_PORT);
  if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    ERROR_EXIT("Could not connect to server\n", 0);

  len = send_data(sock, cfg->dest_filepath, strlen(cfg->dest_filepath), cfg->sneaky_domain);
  wait_for_ack(sock, len);

  return sock;
}

int do_dns_handshake(sender_config *cfg, int *tcp_sock)
{
  char buf[RESPONSE_MAX_LEN] = {0};  // TODO: is it really MAX_Q_LEN? what about starting label len?
  size_t len;
  int query_id = 420;
  prepare_packet(buf, &len, cfg->sneaky_domain, query_id, 0, 1, 0);

  struct sockaddr_in dest;
  char *addr = cfg->ip;
  int sock;
  size_t addr_len = sizeof(dest);
  send_udp4(buf, len, addr, &dest, &sock,  DNS_PORT);

  int received = recvfrom(sock, buf, RESPONSE_MAX_LEN, 0, (struct sockaddr *)&dest, &addr_len);
  if (received <= 0)
    ERROR_EXIT("Could not get response from UDP server\n", EXIT_FAILURE);

  header hdr;
  question q;
  char *domain = retype_parts(buf, &hdr, &q);
  char dns_domain[MAX_QUERY_LEN];
  convert_to_dns_format(dns_domain, cfg->sneaky_domain);
  if (hdr.id != query_id || !hdr.tc || !hdr.qr || strcmp(domain, dns_domain))
    ERROR_EXIT("Response from DND is invalid\n", EXIT_FAILURE);

  if ((*tcp_sock = open_tcp_connection(cfg)) == 0)
    ERROR_EXIT("Could not open TCP connection\n", EXIT_FAILURE);

  return EXIT_SUCCESS;
}

int upload_file(sender_config *cfg, int sock)
{
  size_t domain_len = strlen(cfg->sneaky_domain) + 2;  // 1 for label len and 1 for terminating 0
  char data[EFFECTIVE_CAPACITY(domain_len)];
  char buff[MAX_QUERY_LEN];
  int total = 0, read, sent;

  while ((read = fread(&data, 1, EFFECTIVE_CAPACITY(domain_len), cfg->input)) > 0)
  {
    data[read] = '\0';

    char *encoded = base64_encode(data, read, &read);

    strncpy(buff, encoded, read);
    buff[read] = '\0';
    sent = send_data(sock, buff, read, cfg->sneaky_domain);
    free(encoded);

    total += sent;

    wait_for_ack(sock, sent);
  }
  close(sock);

  printf("\n%d\n", total);
  return EXIT_SUCCESS;
}

int send_to_server(sender_config *cfg)
{
  int tcp_sock;
  if (do_dns_handshake(cfg, &tcp_sock))
    return EXIT_FAILURE;

  printf("cooonnected\n");
  upload_file(cfg, tcp_sock);

  return EXIT_SUCCESS;
}

int main(int args, char *argv[])
{
  sender_config cfg = process_args(args, argv);
  if (!cfg.input)
    return EXIT_FAILURE;

  printf("test\n");
  //if (cfg.input != stdin)
  //  fclose(cfg.input);

  return send_to_server(&cfg);
}
