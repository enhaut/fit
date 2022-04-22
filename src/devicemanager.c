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
#include "string.h"
#include "args_parser.h"

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

void select_device(char *name)
{
  while (devices_ptr)
  {
    if (strcmp(devices_ptr->name, name) == 0)
      break;
    devices_ptr = devices_ptr->next;
  }

  if (strcmp(devices_ptr->name, name))
    devices_ptr = NULL;
}

void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
  printf("%s\n", args);
  return;
}

#define ERROR_RETURN(message)             \
      do{                                 \
          fprintf(stderr, message "\n");  \
          return;                         \
      }while(0)

void capture()
{
  select_device(snifferOptions->inter);
  if (!devices_ptr)
    ERROR_RETURN("Invalid device name!");

  pcap_t *handler = pcap_open_live(devices_ptr->name, BUFSIZ, 1, READING_TIMEOUT, error_buffer);

  if (!handler)
    ERROR_RETURN("Could not open handler!");
  else if (pcap_datalink(handler) != DLT_EN10MB)  // https://www.tcpdump.org/linktypes.html
    ERROR_RETURN("Unsupported interface type");

  if(pcap_loop(handler, snifferOptions->to_sniff, process_packet, NULL))
    fprintf(stderr, "Error during capturing packets\n");
}
