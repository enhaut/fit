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

#ifndef IPK2_SEGMENT_PROCESSORS_H
#define IPK2_SEGMENT_PROCESSORS_H
void process_TCP_segment(void *packet);
void process_UDP_segment(void *packet);
void process_ICMP_segment(void *packet);

#endif // IPK2_SEGMENT_PROCESSORS_H
