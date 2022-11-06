// send_data.c
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 6.11.2022

#include "send_data.h"
#include "dns.h"
#include <string.h>
#include "../sender/dns_sender_events.h"


/**
 * Creates chunks DNS_LABEL_MAX_LENGTH long that are divided by ".".
 *
 * @param data data to be chunked - Needs to be atleast PACKET_BUFFER_SIZE long
 * @param len length of data
 * @return length of chunked array
 */
int chunkize(char *data, size_t len)
{
  int new_len = len;
  for (size_t i = len; i; i--)
  {
    if (i % DNS_LABEL_MAX_LENGTH == 0)
    {
      memmove(&(data[i + 1]), &(data[i]), (len - i + 2));
      // move rest of the string by 2 to the right
      // by 2 because we need to make space for "."
      data[i] = '.';
      new_len++;
    }
  }
  data[new_len] = '\0';
  return new_len;
}

/**
 * Sends data to provided socket.
 *
 * @param sock fd of socket
 * @param data raw data to be sent
 * @param len length of data
 * @param domain sneaky domain
 * @return number of sent bytes
 */
int send_data(int sock, char *data, size_t len, char *domain)
{
  char packet[PACKET_BUFFER_SIZE];
  len = chunkize(data, len);

  strcpy(&(data[len++]), ".");  // add "." behind last chunk of data
  strcpy(&(data[len]), domain);  // add domain
  len += strlen(domain);
  data[len] = '\0';

  prepare_packet(packet, &len, data, 666, 0, 1, 0);

  int sent = send(sock, packet, len, 0);
  return sent;
}
