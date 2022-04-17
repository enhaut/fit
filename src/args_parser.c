/**
 * ipk2
 *
 * @file args_parser.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */
#include "errno.h"
#include <getopt.h>
#include "stdlib.h"
#include "args_parser.h"

sniffer_options_t *get_options_struct()
{
  sniffer_options_t *options = (sniffer_options_t *)malloc(sizeof(sniffer_options_t));
  if (!options)
    ERROR_EXIT("Could not allocate memory\n");

  options->port = 0;
  options->L3 = 0;
  options->L4 = 0;
  options->inter = NULL;
  options->to_sniff = 1;

  return options;
}

unsigned long long get_number(char *raw)
{
  errno = 0;
  char *end;
  unsigned long long number = strtoll(raw, &end, 10);

  if (errno != 0 || end == raw)  // https://man7.org/linux/man-pages/man3/strtol.3.html
  {
    fprintf(stderr, "Invalid number!\n");
    return 0;
  }
  return number;
}

void grateful_exit(sniffer_options_t *options, int code)
{
  free(options);
  exit(code);
}

void set_port(sniffer_options_t *options)
{
  unsigned long long port = get_number(optarg);
  if (port == 0 || port > MAX_PORT)
    grateful_exit(options, EXIT_FAILURE);

  options->port = (int)port;
}

sniffer_options_t *process_args(int argc, char *argv[])
{
  sniffer_options_t *options = get_options_struct();

  static struct option long_options[] =
      {
          {"interface",  required_argument, NULL, 'i'},
          {"port",  required_argument, NULL, 'p'},
          {"tcp",  no_argument, NULL, TCP_BIT},
          {"udp",  no_argument, NULL, UDP_BIT},
          {"icmp",  no_argument, NULL, ICMP_BIT},
          {"arp",  no_argument, NULL, ARP_BIT},
          {0, 0, 0, 0}
      };

  int c, option_index;
  while ((c = getopt_long(argc, argv, "p:i:tun:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'p':
        set_port(options);
        break;
      case 'i':
        options->inter = optarg; // optarg returns ptr to `argv`
        break;
      /* TCP/UDP */
      case 't':
      case 'u':
      case TCP_BIT:
      case UDP_BIT:
        if (c == 't')
          c = TCP_BIT;
        else if (c == 'u')
          c = UDP_BIT;
        options->L4 = (options->L4 | (c-L4_RANGE));
        break;
      case ICMP_BIT:
      case ARP_BIT:
        options->L3 = (options->L3 | (c-L3_RANGE));
        break;
      case 'n':
        options->to_sniff = get_number(optarg);
        if (options->to_sniff > 0)
          break;  // only -n > 0 is valid => let 0 to fall into ERROR_EXIT
      default:
        ERROR_EXIT("Invalid argument\n");
      }
  }

  return options;
}
