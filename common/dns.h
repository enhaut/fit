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


int is_transfer_request(char *domain, char *sneaky_domain);
void resolve(char *raw_query, size_t query_len, struct sockaddr_in6 *requester, int udp_sock);
int receive_dns_packet(int sock, char *buffer);
int prepare_packet(char *dest, uint32_t *len, char *domain, uint16_t id, uint8_t tc, uint16_t qtype, uint16_t qr);
void remove_domain(char *data, char *domain);
int dechunkize(char *data, size_t len);
char *retype_parts(char *raw, header *hdr, question *q);
uint32_t convert_to_dns_format(char *dest, char *domain);

#define ERROR_EXIT(msg, ret) do{printf(msg); return ret;}while(0)




#define DNS_LABEL_MAX_LENGTH (63)
#define MAX_QUERY_LEN (DNS_LABEL_MAX_LENGTH * 4 + 3)
#define PACKET_BUFFER_SIZE (sizeof(header) + sizeof(question) + MAX_QUERY_LEN)
#define RESPONSE_MAX_LEN 512
#define DNS_PORT 53
#define PROXIED_DNS "1.1.1.1"
#define PROXIED_DNS6 "2606:4700:4700::1111"


#define UNEFFECTIVE_CAPACITY(domain_len) (MAX_QUERY_LEN - (domain_len))  // domain_len should include first label len
#define EFFECTIVE_CAPACITY(domain_len) (((UNEFFECTIVE_CAPACITY(domain_len) / 4) * 3) - 3)  // domain_len should include first label len


#define MIN_PACKET_SIZE (sizeof(header) + sizeof(question) + 6) // 6 for 1a2sk\0
#endif // DNS_TUNNELER_DNS_STRUCTS_H
