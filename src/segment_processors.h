/**
 * ipk2
 *
 * @file segment_processors.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */
#include <netinet/ip.h>
#include <netinet/ip6.h>

#ifndef IPK2_SEGMENT_PROCESSORS_H
#define IPK2_SEGMENT_PROCESSORS_H
void process_segment(struct ip *ip_packet);
void process_v6_segment(struct ip6_hdr *ip_packet);
#endif // IPK2_SEGMENT_PROCESSORS_H
