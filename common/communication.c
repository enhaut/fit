/**
 * dns-tunneler
 *
 * @file communication.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "communication.h"

int socket_factory(struct sockaddr_in6 *address, int type)
{
  int generic_socket, t = 1, f = 0;

  if ((generic_socket = socket(AF_INET6, type, 0)) <= 0)
    ERROR_EXIT("socket()", -1);

  if (!address)
    return generic_socket;

  if (setsockopt(generic_socket, SOL_SOCKET, SO_REUSEADDR , &t, sizeof(t)))
    ERROR_EXIT("setsockopt()", -1);

  // Disable IPv6 only => accept both 4 and 6
  if (setsockopt(generic_socket, IPPROTO_IPV6, IPV6_V6ONLY, &f, sizeof(f)))
    ERROR_EXIT("ipv6()", -1);

  if (bind(generic_socket, (struct sockaddr *)address, sizeof(*address)) < 0)
    ERROR_EXIT("bind()", -1);

  return generic_socket;
}
