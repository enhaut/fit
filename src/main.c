/**
 * ipk2
 *
 * @file main.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */
#include "stdlib.h"
#include "args_parser.h"

int main(int argc, char *argv[])
{
  snifferOptions = process_args(argc, argv);

  printf("port: %d\n", options->port);
  printf("interface: %s\n", options->inter);
  printf("to_sniff: %llu\n", options->to_sniff);
  printf("L4: %d\n", options->L4);
  printf("L3: %d\n", options->L3);

  free(options);
  return 0;
}
