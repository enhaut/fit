/**
 * ipk2
 *
 * @file packet_processors.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include <pcap/pcap.h>
#include <sys/types.h>

#ifndef IPK2_PACKET_PROCESSORS_H
#define IPK2_PACKET_PROCESSORS_H

#define CONVERT_ADDR(af, src, dst, len)       \
  do{                                         \
    if (!inet_ntop(af, &(src), dst, len))     \
      sprintf(dst, "INVAL");                  \
  }while(0)

void process_eth_frame(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
#endif // IPK2_PACKET_PROCESSORS_H
