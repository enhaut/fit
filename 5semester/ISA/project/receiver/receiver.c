#include <stdlib.h>

#include "args_parser.h"
#include "connections.h"

/**
 * Entry point of application
 *
 * @param args
 * @param argv
 * @return
 */
int main(int args, char *argv[])
{
  receiver_config cfg = process_args(args, argv);
  if (!cfg.dest_filepath)
    return EXIT_FAILURE;

  return listen_for_queries(&cfg);
}
