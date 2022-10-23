/**
 * dns-tunneler
 *
 * @file dns_structs.c
 *
 * @brief Module implements DNS stuff.
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

/**
 * Function checks whether provided string is already in DNS
 * query format.
 *
 * @param domain
 * @return 1 if string is in required format; 0 otherwise
 */
int is_converted(char *domain)
{
  int counter = 0;

  int label_len = domain[0];
  if (isalnum(label_len))
    return 0;  // there is no label len on domain[0] of first label

  char *c;
  for (c = &(domain[1]); *c; ++c)
  {
    if (isalnum(*c))
      counter++;
    else
    {
      if (label_len != counter)
        return 0;  // length of label does not match real len
      label_len = *c;
      counter = 0;
    }
  }
  return 1;
}

/**
 * Function converts provided from "normal" labels divided
 * by dots notation to DNS format divided by length of following
 * label.
 *
 * @param dest ptr to destination
 * @param domain domain in dot notation with(out) leading .
 * @return length of converted domain
 */
uint32_t convert_to_dns_format(char *dest, char *domain)
{
  size_t len = strlen(domain);

  int converted = is_converted(domain);

  memcpy(&(dest[converted ? 0 : 1]), domain, len);
  // ^^ if domain is already in DNS format, no need to do
  // anything else than just copying it to &(dest[0)
  dest[++len] = '\0';
  if (converted)
    return len;

  char *block_len = dest;
  *block_len = 0;

  // label length counting + setting
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

/**
 * Creates DNS query packet
 *
 * @param dest destination buffer. Needs to be atleast PACKET_BUFFER_SIZE long.
 * @param len ptr to length of buffer (will be set)
 * @param domain data to send
 * @param id ID of query
 * @param tc truncated flag
 * @param qtype type of query
 * @param qr question/response flag
 * @return 0
 */
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
 * Checks whether provided data are chunked or not.
 *
 * @param data
 * @return
 */
int is_last_data_label(char *data)
{
    int remaining_len = (int)strlen((data + 1));

    if (data[0] == remaining_len &&   // label len matches real len
    !strchr(&(data[1]), DNS_LABEL_MAX_LENGTH))
        //  ^^^ check whether there is no DNS_LABEL_MAX_LENGTH long chunk
        return 1;

    return 0;
}

/**
 * Dechunkize data - removes chunk character - "."
 *
 * @param data chunked data
 * @param len total length of data (including separators)
 * @return length of data
 */
int dechunkize(char *data, size_t len)
{
    char *pos = data;
    char *end = data + len;
    int new_len = 0;

    while(*pos)  // in case of invalid packet could overflow
    {
        char *next = pos + 1;
        int label_len = *pos;

        if (label_len == DNS_LABEL_MAX_LENGTH || is_last_data_label(pos))
        {
            if ((pos + label_len) < end)
                next = pos + label_len;  // move to next label length

            memmove(pos, &(pos[1]), end - pos);
            // move rest of the array by one character to the left
            new_len++;
        }
        pos = next;
    }
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

/**
 * Checks whether packet is trying to send query
 * to sneaky domain.
 *
 * @param domain domain from packet including heading label len
 * @param sneaky_domain
 * @return
 */
int is_transfer_request(char *domain, char *sneaky_domain)  // TODO
{
  char buff[MAX_QUERY_LEN];
  convert_to_dns_format(buff, sneaky_domain);

  if (strcmp(domain, buff) == 0)
    return 1;

  return 0;
}

/**
 * Sets start of domain in data to \0 => it removes domain tail.
 *
 * @param data data
 * @param domain sneaky domain to be removed
 */
void remove_domain(char *data, char *domain)
{
    char converted[MAX_QUERY_LEN];
    convert_to_dns_format(converted, domain);

    size_t data_len = strlen(data);
    size_t domain_len = strlen(converted);

    data[(domain_len > data_len) ? 0 : (data_len - domain_len)] = '\0';
    //              ^^^ just to be sure it won't overflow
}

/**
 * Retype parts of DNS packet to corresponding structures.
 *
 * @param raw raw data
 * @param hdr header ptr
 * @param q query ptr
 * @return ptr to start of data (could be label length); NULL on error
 */
char *retype_parts(char *raw, header *hdr, question *q)
{
  ssize_t hdr_len = sizeof(header);
  if (!memcpy(hdr, raw, hdr_len))
    return NULL;

  char *domain = &raw[hdr_len];
  size_t domain_length = strlen(domain);

  // +1 bellow to move behind \0
  if (!memcpy(q, &raw[hdr_len + domain_length + 1], sizeof(question)))
    return NULL;

  hdr->id = htons(hdr->id);

  return domain;
}

/**
 * DNS proxy, just forwards received query to another DNS server.
 * It waits for reply from DNS server and sends reply back to the client.
 *
 * @param raw_query raw data
 * @param query_len raw data length
 * @param requester address struct of requester
 * @param udp_sock socket fd
 */
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


