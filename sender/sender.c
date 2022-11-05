/**
 * dns-tunneler
 *
 * @file sender.c
 *
 * @brief Module implements sender
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "sender.h"
#include "dns_sender_events.h"
#include "args_parser.h"
#include "../common/dns.h"
#include "../common/base64.h"
#include "../common/communication.h"

#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Function is BLOCKING until it receives `size` bytes.
 *
 * @param sock sock to read from
 * @param size size of data to receive
 */
void wait_for_ack(int sock, int size)
{
  char packet[PACKET_BUFFER_SIZE + 1];

  int len, total = 0;
  while (size > 0)
  {
    len = recv(sock, &(packet[total]), size, 0);
    if (len <= 0)
        continue;  // TODO: really?
    total += len;
    size -= len;
  }
  // ^^ needed to block it in this way, because recv is non-blocking on my mac
}

/**
 * Functions establishes TCP connection with DNS server.
 * It also sends initialization packet with filename.
 *
 * @param cfg sender config
 * @return fd of socket
 */
int open_tcp_connection(sender_config *cfg)
{
  int sock, len;
  PREPARE_ADDRESS(addr, cfg->ip, DNS_PORT);

  sock = socket_factory(&addr, SOCK_STREAM, 0);
  if (sock < 0)
    ERROR_EXIT("Could not create TCP socket to server\n", 0);

  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)))
    ERROR_EXIT("Could not connect to server\n", 0);

  len = send_data(sock, cfg->dest_filepath, strlen(cfg->dest_filepath), cfg->sneaky_domain);
  wait_for_ack(sock, len);

  return sock;
}

/**
 * Function initializes "sneaky" connection to DNS server.
 * At first it sends valid DNS query, waits for reply that has
 * truncated flag set on 1, it proceeds with opening TCP connection.
 *
 * @param cfg sender config
 * @param tcp_sock ptr to TCP socket FD
 * @return ERROR_SUCCESS/ERROR_FAILURE
 */
int do_dns_handshake(sender_config *cfg, int *tcp_sock)
{
  char buf[RESPONSE_MAX_LEN] = {0};  // TODO: is it really MAX_Q_LEN? what about starting label len?
  size_t len = 0, addrlen = 0;
  int query_id = 420;
  prepare_packet(buf, &len, cfg->sneaky_domain, query_id, 0, 1, 0);

  PREPARE_ADDRESS(address, cfg->ip, DNS_PORT);

  addrlen = sizeof(address);

  int sock = socket_factory(&address, SOCK_DGRAM, 0);
  len = sendto(sock, (char *)buf, len, 0, (struct sockaddr *)&address, addrlen);
  // send valid query with sneaky domain to server

  if (len < 0)
    ERROR_EXIT("Could not perform dns handshake\n", EXIT_FAILURE);

  //wait for response
  int received = recvfrom(sock, buf, len, 0, (struct sockaddr *)&address, &addrlen);
  if (received <= 0)
    ERROR_EXIT("Could not get response from UDP server\n", EXIT_FAILURE);

  header hdr;
  question q;
  char *domain = retype_parts(buf, &hdr, &q);
  char dns_domain[MAX_QUERY_LEN];
  convert_to_dns_format(dns_domain, cfg->sneaky_domain);
  if (hdr.id != query_id || !hdr.tc || !hdr.qr || strcmp(domain, dns_domain))
    ERROR_EXIT("Response from DNS is invalid\n", EXIT_FAILURE);

  if ((*tcp_sock = open_tcp_connection(cfg)) == 0)
    ERROR_EXIT("Could not open TCP connection\n", EXIT_FAILURE);

  return EXIT_SUCCESS;
}

/**
 * Function uploads file to the server using DNS messages.
 *
 * @param cfg sender config
 * @param sock opened TCP socket
 * @return EXIT_SUCCESS/EXIT_FAILURE
 */
int upload_file(sender_config *cfg, int sock)
{
  size_t domain_len = strlen(cfg->sneaky_domain) + 2;  // 1 for label len and 1 for terminating 0
  char data[EFFECTIVE_CAPACITY(domain_len)];
  char buff[MAX_QUERY_LEN];
  int total = 0, read, sent;
#if MEASURE
#include <sys/time.h>
  struct timeval tv;
  gettimeofday(&tv,NULL);
  printf("acked,time\n0,%ld\n", 1000000 * tv.tv_sec + tv.tv_usec);
#endif

  while ((read = fread(&data, 1, EFFECTIVE_CAPACITY(domain_len), cfg->input)) > 0)
  {
    data[read] = '\0';  // fread does not set \0

    char *encoded = base64_encode(data, read, &read);

    strncpy(buff, encoded, read);  // TODO: could be removed?
    buff[read] = '\0';
    sent = send_data(sock, buff, read, cfg->sneaky_domain);
    free(encoded);

    total += sent;

    wait_for_ack(sock, sent);
#if MEASURE
    gettimeofday(&tv,NULL);
    printf("%d,%ld\n", read, 1000000 * tv.tv_sec + tv.tv_usec);
#endif
  }
  close(sock);

  return EXIT_SUCCESS;
}

/**
 * Function sends the file to the server
 *
 * @param cfg sender config
 * @return ERROR_FAILURE/EXIT_SUCCESS
 */
int send_to_server(sender_config *cfg)
{
  int tcp_sock;
  if (do_dns_handshake(cfg, &tcp_sock))
    return EXIT_FAILURE;

  upload_file(cfg, tcp_sock);

  return EXIT_SUCCESS;
}

/**
 * Entry point to the sender program
 * @param args
 * @param argv
 * @return
 */
int main(int args, char *argv[])
{
  sender_config cfg = process_args(args, argv);
  if (!cfg.input)
    return EXIT_FAILURE;

  //if (cfg.input != stdin)
  //  fclose(cfg.input);

  return send_to_server(&cfg);
}
