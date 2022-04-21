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

#ifndef IPK2_DEVICEMANAGER_H
#define IPK2_DEVICEMANAGER_H

extern pcap_if_t * devices_ptr;

pcap_if_t * getDevices();
void print_device(pcap_if_t *device);
#endif // IPK2_DEVICEMANAGER_H
