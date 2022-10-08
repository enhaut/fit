/**
 * dns-tunneler
 *
 * @file sender.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "sender.h"
#include "dns_sender_events.h"
#include "args_parser.h"

#include <stdlib.h>
#include <stdio.h>


int main(int args, char *argv[])
{
  sender_config cfg = process_args(args, argv);
  if (!cfg.input)
    return EXIT_FAILURE;

  printf("test\n");
  if (cfg.input != stdin)
    fclose(cfg.input);

  return EXIT_SUCCESS;
}
