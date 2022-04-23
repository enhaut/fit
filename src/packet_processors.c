/**
 * ipk2
 *
 * @file packet_processors.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "packet_processors.h"
#include "segment_processors.h"
#include <net/ethernet.h>

#include <time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <linux/if_arp.h>

#define PRINT_MAC(address)                                                \
  do{                                                                     \
    for (int i = 0; i < ETHER_ADDR_LEN; i++)                              \
      printf("%x%s", address[i], ((i+1 < ETHER_ADDR_LEN) ? ":" : ""));    \
  }while(0)

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
}
