// send_data.c
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 6.11.2022

#include "send_data.h"
#include "dns.h"
#include <string.h>
#include "../sender/dns_sender_events.h"
#include "communication.h"


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
int send_data(int sock, char *data, size_t len, char *domain, int *last_id, char *path, char *raw_ip)
{
  char packet[PACKET_BUFFER_SIZE];
  len = chunkize(data, len);
  *last_id = 666;
  //printf("%d\n", *last_id);

  strcpy(&(data[len++]), ".");  // add "." behind last chunk of data
  strcpy(&(data[len]), domain);  // add domain
  len += strlen(domain);
  data[len] = '\0';
  dns_sender__on_chunk_encoded(path, *last_id, data);

  prepare_packet(packet, &len, data, (uint16_t)*last_id, 0, 1, 0);

  int sent = send(sock, packet, len, 0);

  if (raw_ip)  // skip chunk_sent call,this method is also used during handshake
  {
    // just workaround to get type of ip address as provided
    // interface does not implement simple way to handle dual-stack
    // applications. There are lots of this hacky solutions in this tunneler
    // to provide some parameter to interface.
    PREPARE_ADDRESS(addr, raw_ip, DNS_PORT);
    if (ver == AF_INET6)
      dns_sender__on_chunk_sent6(&raw_addr, path, *last_id, len);
    else{
      SOCKADDR6_TO_4(raw_addr);
      dns_sender__on_chunk_sent(raw_v4, path, *last_id, len);
    }
  }

  return sent;
}
