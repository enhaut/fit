/**
 * ipk2
 *
 * @file packet_processors.c
 *
 * @brief Module responsible for processing packets.
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "packet_processors.h"
#include "segment_processors.h"
#include <net/ethernet.h>

#include "ctype.h"
#include <time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <linux/if_arp.h>

// https://stackoverflow.com/a/2409054
void print_time(struct timeval *time)
{
  // 2021-03-19T18:42:52.362+01:00
  char date[30] = "";
  char format[] = "%Y-%m-%dT%H:%M:%S";

  time_t nowtime;
  struct tm *nowtm;
  gettimeofday(time, NULL);
  nowtime = time->tv_sec;
  nowtm = localtime(&nowtime);
  strftime(date, sizeof(date), format, nowtm);
  printf("%s.%ld+01:00", date, time->tv_usec);
}

void process_IP_packet(const u_char *packet)
{
  struct ip *ip_packet = (struct ip *)(packet);
  char src[INET_ADDRSTRLEN] = "";
  char dst[INET_ADDRSTRLEN] = "";

  CONVERT_ADDR(AF_INET, ip_packet->ip_src, src, INET_ADDRSTRLEN);
  CONVERT_ADDR(AF_INET, ip_packet->ip_dst, dst, INET_ADDRSTRLEN);

  printf("src IP: %s\n", src);
  printf("dst IP: %s\n", dst);

  process_segment(ip_packet);
}

void process_IPv6_packet(const u_char *packet)
{
  struct ip6_hdr *ip_packet = (struct ip6_hdr *)(packet);
  char src[INET6_ADDRSTRLEN] = "";
  char dst[INET6_ADDRSTRLEN] = "";

  CONVERT_ADDR(AF_INET6, ip_packet->ip6_src, src, INET6_ADDRSTRLEN);
  CONVERT_ADDR(AF_INET6, ip_packet->ip6_dst, dst, INET6_ADDRSTRLEN);

  printf("src IP: %s\n", src);
  printf("dst IP: %s\n", dst);

  process_v6_segment(ip_packet);
}

void process_ARP_packet(const u_char *packet)
{
  struct arphdr *arp_packet = (struct arphdr*)(packet);
  printf("opcode: %d\n", arp_packet->ar_op);
}

/**
 * @brief Function tries to print bytes as a characters.
 * If character is not printable, it replaces it by '.' character.
 *
 * @param line array containing characters of line to print
 * @param chars # of chars to print
 */
void dump_line(const u_char *line, unsigned int chars)
{
  printf("\t");
  for (unsigned int i = 0; i < chars; i++)
    printf("%c", (isprint(line[i])) ? line[i] : '.');

  printf("\n");
}

/**
 * @brief Function hexdumps ethernet frame content.
 *
 * @param frame raw bytes of frame
 * @param size size of frame in bytes
 */
void dump_frame(const u_char *frame, unsigned int size)
{
  printf("\n");
  unsigned char line_buffer[LINE_LENGTH] = {0};

  for (unsigned int i = 0; i < size; i++)
  {
    line_buffer[i % LINE_LENGTH] = frame[i];
    // ^ store bytes to dump it at the end of line

    char line[9] = "";
    // line is printed every 17. byte, which is start of new line with bytes =>
    // prints it before 17.byte (indexing from 1)
    if (i % LINE_LENGTH == 0)
      sprintf(line, "0x%04X:\t", i);

    printf("%s%02x ", line, (frame[i]));

    // (dump line after every 15. byte (because of indexing from 0)) OR
    // (at the end of frame)
    if ((i+1)%LINE_LENGTH == 0 || i+1 == size)
      dump_line(line_buffer,
                (((size - i) > LINE_LENGTH) || i == LINE_LENGTH) ?
                                            LINE_LENGTH :
                                        ((i % LINE_LENGTH)+1)
                );
      // ^ print up to LINE_LENGTH bytes if available otherwise dump
      // remaining bytes up to LINE_LENGTH-1;i is indexed from 0^
  }
}

/***
 * @brief Entrypoint function for processing ethernet frame.
 * It calls processing function at higher layers.
 *
 * @param args
 * @param header eth frame header
 * @param packet whole frame
 */
void process_eth_frame(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
  struct ether_header *eth_header = (struct ether_header *) packet;

  printf("timestamp: ");
  print_time(&(header->ts));  // TODO
  printf("\nsrc MAC: ");
  PRINT_MAC(eth_header->ether_shost);
  printf("\ndst MAC: ");
  PRINT_MAC(eth_header->ether_dhost);
  printf("\nframe length: %d bytes\n", header->len);


  packet += 14;  // move ptr behind end of ETH frame header
  switch (ntohs(eth_header->ether_type))
  {
    case ETHERTYPE_IP:
      process_IP_packet(packet);
      break;
    case ETHERTYPE_IPV6:
      process_IPv6_packet(packet);
      break;
    case ETHERTYPE_ARP:
    case ETHERTYPE_REVARP:
      process_ARP_packet(packet);
      break;
    case ETHERTYPE_VLAN:  // TODO
      break;
  }
  dump_frame(packet, header->len);
}
