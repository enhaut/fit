/**
 * ipk2
 *
 * @file devicemanager.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "stdlib.h"

#include "devicemanager.h"
pcap_if_t * devices_ptr = NULL;
char error_buffer[PCAP_ERRBUF_SIZE] = {0};

pcap_if_t * getDevices()
{
  pcap_if_t *dev_ptr = (pcap_if_t *)malloc(sizeof(pcap_if_t));
  if (!dev_ptr)
    return NULL;

  pcap_findalldevs(&dev_ptr, error_buffer);
  if (!dev_ptr)
    return NULL;

  return dev_ptr;
}

void print_device(pcap_if_t *device)
{

  // assignment does not specify what does "active" interface mean, so it prints
  // all connected devices and loopbacks.
  if ((!(device->flags & PCAP_IF_CONNECTION_STATUS_DISCONNECTED) &&
       (device->flags & PCAP_IF_CONNECTION_STATUS_CONNECTED)) ||
      device->flags & PCAP_IF_LOOPBACK)
    printf("%s\n", device->name);

  if (device->next)
    print_device(device->next);  // recursion goes brrrrr
  // ^ tested at server with 24 devices, if it fails at your machine, adjust stack size. see `man ulimit`
}
