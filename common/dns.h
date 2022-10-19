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



void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
) {
  // Silently ignore silly per-line values.

  if (perLine < 4 || perLine > 64) perLine = 16;

  int i;
  unsigned char buff[perLine+1];
  const unsigned char * pc = (const unsigned char *)addr;

  // Output description if given.

  if (desc != NULL) printf ("%s:\n", desc);

  // Length checks.

  if (len == 0) {
    printf("  ZERO LENGTH\n");
    return;
  }
  if (len < 0) {
    printf("  NEGATIVE LENGTH: %d\n", len);
    return;
  }

  // Process every byte in the data.

  for (i = 0; i < len; i++) {
    // Multiple of perLine means new or first line (with line offset).

    if ((i % perLine) == 0) {
      // Only print previous-line ASCII buffer for lines beyond first.

      if (i != 0) printf ("  %s\n", buff);

      // Output the offset of current line.

      printf ("  %04x ", i);
    }

    // Now the hex code for the specific character.

    printf (" %02x", pc[i]);

    // And buffer a printable ASCII character for later.

    if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
      buff[i % perLine] = '.';
    else
      buff[i % perLine] = pc[i];
    buff[(i % perLine) + 1] = '\0';
  }

  // Pad out last line if not exactly perLine characters.

  while ((i % perLine) != 0) {
    printf ("   ");
    i++;
  }

  // And print the final ASCII buffer.

  printf ("  %s\n", buff);
}

#define DNS_LABEL_MAX_LENGTH (63)
#define MAX_QUERY_LEN (DNS_LABEL_MAX_LENGTH * 4 + 3)
#define PACKET_BUFFER_SIZE (sizeof(header) + sizeof(question) + MAX_QUERY_LEN)
#define RESPONSE_MAX_LEN 512
#define DNS_PORT 53
#endif // DNS_TUNNELER_DNS_STRUCTS_H
