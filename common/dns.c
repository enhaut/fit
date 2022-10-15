/**
 * dns-tunneler
 *
 * @file dns_structs.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>

#include "dns.h"

int send_udp4(char *data, size_t len, const char *addr, struct sockaddr_in *dest, int *s, int port)
{
  int sock;

  if (port == 53)
  {
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s)
      *s = sock;
  } else
    sock = *s;

  dest->sin_addr.s_addr = inet_addr(addr);
  dest->sin_family = AF_INET;
  dest->sin_port = htons(port);

  return sendto(sock, (char *)data, len, 0, (struct sockaddr *)dest, sizeof(*dest));
}

int is_transfer_request(char *domain, char *sneaky_domain)  // TODO
{
  char buff[MAX_QUERY_LEN];
  convert_to_dns_format(buff, sneaky_domain);

  if (strcmp(domain, buff) == 0)
    return 1;

  return 0;
}

void resolve(char *raw_query, size_t query_len, struct sockaddr_in6 *requester, int udp_sock)
{
  int proxy_sock, sent;
  struct sockaddr_in dns_srv, me;
  char buffer[RESPONSE_MAX_LEN];

  socklen_t len = sizeof(me);

  // forward request to real dns server
  sent = send_udp4(raw_query, query_len, "1.1.1.1", &dns_srv, &proxy_sock, 53);
  if (sent <= 0)
    return;

  // get response from real dns
  sent = recvfrom(proxy_sock, buffer, RESPONSE_MAX_LEN, 0, ( struct sockaddr *) &me, &len);
  if (sent <= 0)
    return;

  // send response back to requester
  sendto(udp_sock, buffer, sent, 0, (struct sockaddr *)requester, sizeof(*requester));
}


