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

void process_eth_frame(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
  struct ether_header *eth_header = (struct ether_header *) packet;

  int i;
  u_char *ptr; /* printing out hardware header info */
  ptr = eth_header->ether_dhost;
  i = ETHER_ADDR_LEN;
  printf(" Destination Address:  ");
  do{
    printf("%s%x",(i == ETHER_ADDR_LEN) ? " " : ":",*ptr++);
  }while(--i>0);
  printf("\n");

  switch (ntohs(eth_header->ether_type))
  {
    case ETHERTYPE_IP:
      printf("IP\n");
      break;
    case ETHERTYPE_ARP:
      printf("ARP\n");
      break;
    }
}
