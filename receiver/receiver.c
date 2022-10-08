#include <stdlib.h>
#include <stdio.h>

#include "args_parser.h"

int main(int args, char *argv[])
{
  receiver_config cfg = process_args(args, argv);
  if (!cfg.dest_filepath)
    return EXIT_FAILURE;

  printf("%s\n%saaa\n", cfg.sneaky_domain, cfg.dest_filepath);

  return EXIT_SUCCESS;
}
