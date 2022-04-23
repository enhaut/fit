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
#include <net/ethernet.h>
#include <time.h>

#include <netinet/ip.h>


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
  struct ip *ip_packet = (struct ip *)(packet + 14);
  printf("src IP: %s\n", inet_ntoa(ip_packet->ip_src));
  printf("dst IP: %s\n", inet_ntoa(ip_packet->ip_dst));
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


  switch (ntohs(eth_header->ether_type))
  {
    case ETHERTYPE_IP:
      process_IP_packet(packet);
      break;
    case ETHERTYPE_IPV6:
      printf("IPv6\n");
      break;
    case ETHERTYPE_ARP:
      printf("ARP\n");
      break;
    }
}
