/**
 * ipk2
 *
 * @file devicemanager.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "stdbool.h"
#include <pcap/pcap.h>
#include <sys/types.h>

#include "args_parser.h"

#ifndef IPK2_DEVICEMANAGER_H
#define IPK2_DEVICEMANAGER_H

extern pcap_if_t * devices_ptr;

pcap_if_t * getDevices();
void print_device(pcap_if_t *device);

#define READING_TIMEOUT 1000
typedef void ( *handler_func_t)(u_char *, const struct pcap_pkthdr*, const u_char *);
void capture();
#endif // IPK2_DEVICEMANAGER_H
