/**
 * ipk2
 *
 * @file segment_processors.c
 *
 * @brief Module responsible for processing segments.
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "segment_processors.h"
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include "stdio.h"

void process_TCP_segment(void *packet)
{
  struct tcphdr *segment = (struct tcphdr *)packet;
  printf("src port: %d\n", ntohs(segment->th_sport));
  printf("dst port: %d\n", ntohs(segment->th_dport));
  printf("syn: %d, ack: %d\n", segment->syn, segment->ack);
}
void process_UDP_segment(void *packet)
{
  struct udphdr *segment = (struct udphdr *)packet;
  printf("src port: %d\n", ntohs(segment->uh_sport));
  printf("dst port: %d\n", ntohs(segment->uh_dport));
}

void process_ICMP_segment(void *packet)
{
  struct icmphdr *segment = (struct icmphdr *)packet;
  printf("type: ");
  switch (segment->type)
  {
    case ICMP_ECHOREPLY:      printf("ECHO_REPLY"); break;
    case ICMP_DEST_UNREACH:   printf("DEST_UNREACH"); break;
    case ICMP_SOURCE_QUENCH:  printf("SOURCE_QUENCH"); break;
    case ICMP_REDIRECT:       printf("REDIRECT"); break;
    case ICMP_ECHO:           printf("ECHO"); break;
    case ICMP_TIME_EXCEEDED:  printf("TIME_EXCEEDED"); break;
    case ICMP_PARAMETERPROB:  printf("PARAMETERPROB"); break;
    case ICMP_TIMESTAMP:      printf("TIMESTAMP_REQ"); break;
    case ICMP_TIMESTAMPREPLY: printf("TIMESTAMP_REPLY"); break;
    case ICMP_INFO_REQUEST:   printf("INFO_REQUEST"); break;
    case ICMP_INFO_REPLY:     printf("INFO_REPLY"); break;
    case ICMP_ADDRESS:        printf("ADDRESS_MASK_REQ"); break;
    case ICMP_ADDRESSREPLY:   printf("ADDRESS_MASK_REPLY"); break;
    }
    printf("(%d)\n", segment->type);
}

void process_segment(struct ip *ip_packet)
{
  void *segment_start = (void *)(ip_packet) + ip_packet->ip_hl*4;
  if (ip_packet->ip_p == IPPROTO_TCP)
    process_TCP_segment(segment_start);
  else if(ip_packet->ip_p == IPPROTO_UDP)
    process_UDP_segment(segment_start);
  else if (ip_packet->ip_p == IPPROTO_ICMP)
    process_ICMP_segment(segment_start);
}

void process_v6_segment(struct ip6_hdr *ip_packet)
{
  void *segment_start = (void *)(ip_packet) + 40;  // move ptr to start of extension hdr

  if (ip_packet->ip6_nxt == IPPROTO_TCP)
    process_TCP_segment(segment_start);
  else if(ip_packet->ip6_nxt == IPPROTO_UDP)
    process_UDP_segment(segment_start);
  else if (ip_packet->ip6_nxt == IPPROTO_ICMP)
    process_ICMP_segment(segment_start);
}
