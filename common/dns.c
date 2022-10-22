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
#include "base64.h"

int is_converted(char *domain)
{
  int counter = 0;

  int label_len = domain[0];
  if (isalnum(label_len))
    return 0;

  char *c;
  for (c = &(domain[1]); *c; ++c)
  {
    if (isalnum(*c))
      counter++;
    else
    {
      if (label_len != counter)
        return 0;
      label_len = *c;
      counter = 0;
    }
  }
  return 1;
}

uint32_t convert_to_dns_format(char *dest, char *domain)
{
  size_t len = strlen(domain);

  int converted = is_converted(domain);

  memcpy(&(dest[converted ? 0 : 1]), domain, len);
  dest[++len] = '\0';
  if (converted)
    return len;

  char *block_len = dest;
  *block_len = 0;

  for (size_t i = 1; i < len; i++)
  {
    if (dest[i] == '.' || dest[i] == '/')
    {
      block_len = &(dest[i]);
      *block_len = 0;
    } else
      (*block_len)++;
  }
  return len + 1;  // +1 for first label length
}

//TODO: timeouts???
#define READ(bf, size) do{                          \
    read = recv(sock, bf, size, 0);                 \
    if (read <= 0)                                  \
    {                                               \
        (bf)[0] = '\0';                             \
        if (read == 0)                              \
            return -1;  /* connection closed */     \
        else  /* some kind of error  */             \
            ERROR_EXIT("Could not get data\n", -1); \
    }                                               \
    total += read;                                  \
}while(0)
int receive_dns_packet(int sock, char *buffer)
{
    int total = 0, read;
    READ(buffer, sizeof(header));

    while ((total + 1) < PACKET_BUFFER_SIZE)  // TODO might overflow
    {
        READ(&(buffer[total]), 1);

        if (buffer[total-1] == '\0')
            break;
    }

    READ(&(buffer[total]), sizeof(question));

    return total;
}
#undef READ

int prepare_packet(char *dest, uint32_t *len, char *domain, uint16_t id, uint8_t tc, uint16_t qtype, uint16_t qr)
{
  header header = {0};
  header.id = htons(id);
  header.rd = 1;
  header.tc = tc;
  header.qr = qr;

  header.qdcount = htons(1);
  *len = sizeof(header);
  memcpy(dest, &header, *len);

  *len += convert_to_dns_format(dest + *len, domain);

  question question;
  question.qtype = htons(qtype);
  question.qclass = htons(1);

  memcpy(dest + *len, &question, sizeof(question));
  *len += sizeof(question);

  return 0;
}

int chunkize(char *data, size_t len)
{
    int new_len = len;
    for (size_t i = len; i; i--)
    {
        if (i % DNS_LABEL_MAX_LENGTH == 0)
        {
            memmove(&(data[i + 1]), &(data[i]), (len - i + 2));
            data[i] = '.';
            new_len++;
        }
    }
    return new_len;
}

int dechunkize(char *data, size_t len)
{
    char *pos = data;
    char *end = data + len;
    int new_len = 0;

    while(*pos)  // in case of invalid packet could overflow
    {
        if (!isalnum(*pos) || *pos != '+' || *pos != '=' || *pos != '-')
        {
            memmove(pos, &(pos[1]), end - pos);
            new_len++;
        }
        pos++;
    }
    return new_len;
}

int send_data(int sock, char *data, size_t len, char *domain)
{
    char packet[PACKET_BUFFER_SIZE];
    //len = chunkize(data, len);

    strcpy(&(data[len++]), ".");
    strcpy(&(data[len]), domain);
    len += strlen(domain);
    data[len] = '\0';

    prepare_packet(packet, &len, data, 666, 0, 1, 0);

    return send(sock, packet, len, 0);
}

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

void remove_domain(char *data, char *domain)
{
    char converted[MAX_QUERY_LEN];
    convert_to_dns_format(converted, domain);

    size_t data_len = strlen(data);
    size_t domain_len = strlen(converted);

    data[(domain_len > data_len) ? 0 : (data_len - domain_len)] = '\0';
    //              ^^^ just to be sure it won't overflow
}

char *retype_parts(char *raw, header *hdr, question *q)
{
  ssize_t hdr_len = sizeof(header);
  if (!memcpy(hdr, raw, hdr_len))
    return NULL;

  char *domain = &raw[hdr_len];
  size_t domain_length = strlen(domain);

  if (!memcpy(q, &raw[hdr_len + domain_length + 1], sizeof(question)))
    return NULL;

  hdr->id = htons(hdr->id);

  return domain;
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


