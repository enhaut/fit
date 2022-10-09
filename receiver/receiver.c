#include <stdlib.h>

#include "args_parser.h"
#include "connections.h"


int main(int args, char *argv[])
{
  // signal(SIGINT, sig_handler);  // TODO

  receiver_config cfg = process_args(args, argv);
  if (!cfg.dest_filepath)
    return EXIT_FAILURE;

  return listen_for_queries(&cfg);
}
