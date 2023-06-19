/**
 * dns-tunneler
 *
 * @file args_parser.c
 *
 * @brief Module implements argument parsing
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "args_parser.h"
#include "../common/dns.h"
#include <string.h>

/**
 * Simple function that parses arguments.
 *
 * @param args number of arguments
 * @param argv arguments
 * @return receiver config structure
 */
receiver_config process_args(int args, char *argv[])
{
  receiver_config cfg = {0};
  if (args == 3)
  {
    if (strlen(argv[1]) < DOMAIN_MAX_LEN &&
        strlen(argv[2]) < MAX_DATA_LEN/2)
    {
      cfg.sneaky_domain = argv[1];
      cfg.dest_filepath = argv[2];
    }else
      printf("Arguments are too long!\n");
  }else
    printf("Invalid arguments!\n");

  return cfg;
}

