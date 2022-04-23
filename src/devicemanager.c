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
#include "packet_processors.h"
#include "devicemanager.h"
#include "args_parser.h"

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

handler_func_t get_handler_function(pcap_t *handler)
{
  switch (pcap_datalink(handler))
  {
    case DLT_EN10MB:
      return process_eth_frame;
    default:
      return NULL;
  }
}

void set_rules(char *rules_str)
{
  int pos = 0;

  // add parentheses only if L4 or L3 filter is set
  if (snifferOptions->L4 || snifferOptions->L3)
  {
    pos += sprintf(rules_str + pos, "(");
    if (snifferOptions->L4 & (TCP_BIT - L4_RANGE))
      ADD_RULE(rules_str, " or ", "tcp", NULL);

    if (snifferOptions->L4 & (UDP_BIT - L4_RANGE))
      ADD_RULE(rules_str, " or ", "udp", NULL);

    if (snifferOptions->L3 & (ICMP_BIT - L3_RANGE))
      ADD_RULE(rules_str, " or ", "icmp or icmp6", NULL);

    if (snifferOptions->L3 & (ARP_BIT - L3_RANGE))
      ADD_RULE(rules_str, " or ", "arp or rarp", NULL);
    pos += sprintf(rules_str + pos, ")");
  }

  if (snifferOptions->port)
    ADD_RULE(rules_str, " and ", "port %d", snifferOptions->port);
}

int set_filter(pcap_t *handler)
{
  char rules[MAX_RULES_LENGTH] = {0};
  set_rules(rules);
  DEBUG("BPF: %s", rules);

  struct bpf_program bpf;
  if (pcap_compile(handler, &bpf, rules, BPF_OPTIMIZATION, PCAP_NETMASK_UNKNOWN))
    return EXIT_FAILURE;

  if (pcap_setfilter(handler, &bpf))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

void capture()
{
  select_device(snifferOptions->inter);
  if (!devices_ptr)
    ERROR_RETURN("Invalid device name!");

  pcap_t *handler = pcap_open_live(devices_ptr->name, BUFSIZ, 1, READING_TIMEOUT, error_buffer);

  if (!handler)
    ERROR_RETURN("Could not open handler!");

  handler_func_t handler_func = get_handler_function(handler);
  if (!handler_func)
    ERROR_RETURN("Unsupported interface type!");

  if (set_filter(handler))
    ERROR_RETURN("Invalid combination of filters (arp/icmp does NOT use ports)!");

  // TODO: maybe implement multiple looping in case, to_sniff is bigger than INT_MAX
  if(pcap_loop(handler, (int)(snifferOptions->to_sniff), handler_func, NULL))
    fprintf(stderr, "Error during capturing packets\n");

  pcap_close(handler);
}
