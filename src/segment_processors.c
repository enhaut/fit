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
#include <netinet/in.h>
#include "stdio.h"

void process_TCP_segment(void *packet)
{
  struct tcphdr *segment = (struct tcphdr *)packet;
  printf("src port: %d\n", ntohs(segment->th_sport));
  printf("dst port: %d\n", ntohs(segment->th_dport));
}
