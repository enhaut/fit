/**
 * dns-tunneler
 *
 * @file args_parser.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "args_parser.h"
#include <string.h>

#define COMPARE_STR(a, b) (strcmp(a, b) == 0)
int defined_u_param(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
    if (COMPARE_STR(argv[i], "-u"))
      return 1;

  return 0;
}

#define ERROR_RETURN(msg, ret) do{printf(msg); return ret;}while(0)
FILE *input_file(int argc, char *argv[])
{
  FILE *ptr = NULL;

  if (argc == 6 || argc == 4)
  {
    ptr = fopen(argv[argc-1], "r");
    if (!ptr)
      ERROR_RETURN("Could not open file!\n", ptr);
    printf("opened\n");
  }else
    ptr = stdin;

  return ptr;
}

sender_config process_args(int args, char *argv[])
{
  sender_config cfg = {0};
  int u_param = defined_u_param(args, argv);

  if (
      args < 3 ||  // at least dns_sender {BASE_HOST} {DST_FILEPATH}
      (u_param && (args < 5 || args > 6)) ||  // in case of -u used, allowed length of args is 5 or 6
      (!u_param && args > 4)  // in case of not using -u, allowed lengths are 3 or 4
  )
    ERROR_RETURN("Invalid arguments\n", cfg);

  for (int i = 1; i < args; i++)
  {
    if (COMPARE_STR(argv[i], "-u"))
    {
      // TODO: process -u parameter without space between
      if ((i + 1) >= args)
        ERROR_RETURN("Invalid -u value\n", cfg);

      strcpy(cfg.ip, argv[++i]);
    }else{
      if (cfg.sneaky_domain[0] == '\0')
        strcpy(cfg.sneaky_domain, argv[i]);
      else if (cfg.dest_filepath[0] == '\0')
        strcpy(cfg.dest_filepath, argv[i]);
      else if (!cfg.input)
        printf("FILE: %s\n", argv[i]);
    }
  }
  cfg.input = input_file(args, argv);
  return cfg;
}

