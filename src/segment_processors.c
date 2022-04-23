/**
 * ipk2
 *
 * @file segment_processors.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "segment_processors.h"
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include "stdio.h"

void process_TCP_segment(void *packet)
{
  struct tcphdr *segment = (struct tcphdr *)packet;
  printf("src port: %d\n", ntohs(segment->th_sport));
  printf("dst port: %d\n", ntohs(segment->th_dport));
}
void process_UDP_segment(void *packet)
{
  struct udphdr *segment = (struct udphdr *)packet;
  printf("src port: %d\n", ntohs(segment->uh_sport));
  printf("dst port: %d\n", ntohs(segment->uh_dport));
}
