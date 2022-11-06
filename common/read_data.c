// read_data.c
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 6.11.2022

#include "read_data.h"
#include "dns.h"

/**
 * Function reads sizeof(header) + sizeof(question) + sizeof domain
 * from provided socket. Domain end is determined by trailing \0.
 *
 * @param sock fd to socket
 * @param buffer destination buffer. Needs to be at least PACKET_BUFFER size long.
 * @return number of read bytes
 */
int receive_dns_packet(int sock, char *buffer)
{
  int total = 0, read;
  READ(buffer, sizeof(header));

  while ((total + 1) < PACKET_BUFFER_SIZE)
  {
    READ(&(buffer[total]), 1);

    if (buffer[total-1] == '\0')
      break;
  }

  READ(&(buffer[total]), sizeof(question));

  return total;
}
#undef READ
