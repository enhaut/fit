/**
 * ipk2
 *
 * @file args_parser.c
 *
 * @brief Module responsible for arguments parsing.
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */
#include "errno.h"
#include "stdbool.h"
#include <getopt.h>
#include "stdlib.h"
#include "args_parser.h"
#include "devicemanager.h"

sniffer_options_t *snifferOptions = NULL;

/**
 * @brief Allocates memory for `sniffer_options_t` struct, which holds
 * sniffer options. FOr example if sniffer should capture ICMP/TCP/... packets.
 *
 * @return ptr to `sniffer_options_t` struct
 */
sniffer_options_t *get_options_struct()
{
  sniffer_options_t *options = (sniffer_options_t *)malloc(sizeof(sniffer_options_t));
  if (!options)
    ERROR_EXIT("Could not allocate memory\n");

  // set default values
  options->port = 0;
  options->L3 = 0;
  options->L4 = 0;
  options->inter = NULL;
  options->to_sniff = 1;

  return options;
}

/**
 * @brief Function parses number from string. In case of invalid
 * number, it returns 0. `0` is OK to return, function is used to
 * parse ports which have to be bigger than 0.
 *
 * @param raw
 * @return converted number
 */
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

/**
 * @brief Function releases all allocated/opened resources
 * and kills the program.
 *
 * @param options ptr to sniffer options
 * @param code exit code
 */
void grateful_exit(sniffer_options_t *options, int code)
{
  if (handler)
    pcap_close(handler);
  free(options);
  exit(code);
}

/**
 * @brief Function sets port from argument to sniffer options.
 * @param options
 */
void set_port(sniffer_options_t *options)
{
  unsigned long long port = get_number(optarg);
  if (port == 0 || port > MAX_PORT)
    grateful_exit(options, EXIT_FAILURE);

  options->port = (int)port;
}

/**
 * @brief Taken from https://stackoverflow.com/a/69177115 by "larsewi"
 */
#define OPTIONAL_ARGUMENT_IS_PRESENT \
    ((optarg == NULL && optind < argc && argv[optind][0] != '-') \
     ? (bool) (optarg = argv[optind++]) \
     : (optarg != NULL))

/**
 * @brief Function process arguments of program.
 * There are 2 types of arguments. Long and short both are parsed
 * using function `getopt_long()`.
 * @param argc
 * @param argv
 * @return ptr to struct `sniffer_options_t` which contains sniffer options
 */
sniffer_options_t *process_args(int argc, char *argv[])
{
  if (argc == 1)
  {
    fprintf(stderr, "Usage:\n"
                    "\t./ipk-sniffer [-i | -i interface | --interface interface] {-p port} {[--tcp|-t] [--udp|-u] [--arp] [--icmp] } {-n num}\n");
    exit(1);  // nothing is allocated
  }

  sniffer_options_t *options = get_options_struct();

  static struct option long_options[] =
      {
          {"interface",  optional_argument, NULL, 'i'},
          {"port",  required_argument, NULL, 'p'},
          {"tcp",  no_argument, NULL, TCP_BIT},
          {"udp",  no_argument, NULL, UDP_BIT},
          {"icmp",  no_argument, NULL, ICMP_BIT},
          {"arp",  no_argument, NULL, ARP_BIT},
          {0, 0, 0, 0}
      };

  options->inter = argv[0];  // argv[0] is always present, it could be used to check if required argument -i was used
  int c, option_index;
  while ((c = getopt_long(argc, argv, "p:i::tun:", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'p':
        set_port(options);
        break;
      case 'i':
        if (OPTIONAL_ARGUMENT_IS_PRESENT)
          options->inter = optarg; // optarg returns ptr to `argv`
        else
          options->inter = NULL;
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

  if (options->inter == argv[0])
    ERROR_EXIT("Argument -i is required!\n");

  return options;
}
