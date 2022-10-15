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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int open_tcp_connection(sender_config *cfg)
{
  int sock;
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

  char buf[MAX_QUERY_LEN] = {0};
  int query_id = 69;
  int len;
  prepare_packet(buf, &len, cfg->dest_filepath, query_id, 0, 1, 0);
  write(sock, buf, len);

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

int send_to_server(sender_config *cfg)
{
  int tcp_sock;
  if (do_dns_handshake(cfg, &tcp_sock))
    return EXIT_FAILURE;

  printf("cooonnected\n");

  return EXIT_SUCCESS;
}

int main(int args, char *argv[])
{
  sender_config cfg = process_args(args, argv);
  if (!cfg.input)
    return EXIT_FAILURE;

  printf("test\n");
  if (cfg.input != stdin)
    fclose(cfg.input);

  return send_to_server(&cfg);
}
