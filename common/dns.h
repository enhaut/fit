/**
 * dns-tunneler
 *
 * @file dns_structs.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef DNS_TUNNELER_DNS_STRUCTS_H
#define DNS_TUNNELER_DNS_STRUCTS_H

typedef struct {
  uint16_t id;

  uint8_t rd:1;
  uint8_t tc:1;
  uint8_t aa:1;
  uint8_t opcode:4;
  uint8_t qr:1;
  
  uint8_t rcode:4;
  uint8_t cd:1;
  uint8_t ad:1;
  uint8_t z:1;
  uint8_t ra:1;

  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
}header;


typedef struct {
  uint16_t qtype;
  uint16_t qclass;
}question;


int send_udp4(char *data, size_t len, const char *addr, struct sockaddr_in *dest, int *s, int port);
int is_transfer_request(char *domain, char *sneaky_domain);
void resolve(char *raw_query, size_t query_len, struct sockaddr_in6 *requester, int udp_sock);
int prepare_packet(char *dest, uint32_t *len, char *domain, uint16_t id, uint8_t tc, uint16_t qtype, uint16_t qr);
char *retype_parts(char *raw, header *hdr, question *q);
uint32_t convert_to_dns_format(char *dest, char *domain);

#define ERROR_EXIT(msg, ret) do{printf(msg); return ret;}while(0)

#define DNS_LABEL_MAX_LENGTH (63)
#define MAX_QUERY_LEN (DNS_LABEL_MAX_LENGTH * 4 + 1)
#define PACKET_BUFFER_SIZE (sizeof(header) + sizeof(question) + MAX_QUERY_LEN)
#define RESPONSE_MAX_LEN 512
#define DNS_PORT 53
#endif // DNS_TUNNELER_DNS_STRUCTS_H
