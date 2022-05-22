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

/**
 * @brief The idea is inspired by
 * http://yuba.stanford.edu/~casado/pcap/section2.html.
 */
#define PRINT_MAC(address)                                                \
  do{                                                                     \
    for (int i = 0; i < ETHER_ADDR_LEN; i++)                              \
      printf("%02x%s", address[i], ((i+1 < ETHER_ADDR_LEN) ? ":" : ""));    \
  }while(0)

/**
 * @brief Macro that converts number representation of IP address
 * to human-readable address.
 */
#define CONVERT_ADDR(af, src, dst, len)       \
  do{                                         \
    if (!inet_ntop(af, &(src), dst, len))     \
      sprintf(dst, "INVAL");                  \
  }while(0)
#define LINE_LENGTH 16

void process_eth_frame(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
#endif // IPK2_PACKET_PROCESSORS_H
