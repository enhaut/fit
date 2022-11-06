/**
 * dns-tunneler
 *
 * @file args_parser.c
 *
 * @brief Argument processor module
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "args_parser.h"
#include "../common/dns.h"
#include <string.h>
#include <stdlib.h>

#define COMPARE_STR(a, b) (strcmp(a, b) == 0)
/**
 * Checks whether there is -u argument in arguments
 *
 * @param argc number of args
 * @param argv arguments
 * @return 1 if so; 0 otherwise
 */
int defined_u_param(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
    if (COMPARE_STR(argv[i], "-u"))
      return 1;

  return 0;
}

#define ERROR_RETURN(msg, ret) do{printf(msg); return ret;}while(0)
/**
 * Opens input file or sets ptr to stdin if there no any.
 *
 * @param argc number of arguments
 * @param argv arguments
 * @return ptr to input file
 */
FILE *input_file(int argc, char *argv[])
{
  FILE *ptr = NULL;

  if (argc == 6 || argc == 4)
  {
    ptr = fopen(argv[argc-1], "rb");
    if (!ptr)
      ERROR_RETURN("Could not open file!\n", ptr);
    printf("opened\n");
  }else
    ptr = stdin;

  return ptr;
}

/**
 * Returns default DNS server from /etc/resolv.conf file.
 * If there is some problem, it closes cfg.input file to
 * indicate it.
 *
 * @param cfg sender config
 * @return
 */
__attribute__((unused)) void *get_default_dns_server(sender_config *cfg)
{
  if (cfg->ip[0])  // -u has been used
    return NULL;

  FILE *resolv = fopen("/etc/resolv.conf", "r");
  if (!resolv)
    ERROR_RETURN("Could not read /etc/resolv.conf, use -u instead\n", NULL);

  char *line;
  size_t _;
  char line_prefix[] = "nameserver ";  // line with dns srvs starts with this
  size_t prefix_len = strlen(line_prefix);

  while ((getline(&line, &_, resolv)) != EOF && !cfg->ip[0])
  {
    if ((strncmp(line, line_prefix, prefix_len) == 0))
      strncpy(cfg->ip, &line[prefix_len], strlen(&line[prefix_len]) - 1);
  }

  free(line);
  fclose(resolv);

  if (!cfg->ip[0]) {
    printf("Could not get DNS server IP, use -u instead\n");
    if (cfg->input != stdin)
      fclose(cfg->input);
    cfg->input = NULL;  // mark config as invalid
  }
  return NULL;
}

/**
 * Function parses arguments.
 *
 * @param args number of arguments
 * @param argv arguments
 * @return sender_config structure
 */
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
      if ((i + 1) >= args)
        ERROR_RETURN("Invalid -u value\n", cfg);

      strcpy(cfg.ip, argv[++i]);
    }else{
      if (!cfg.sneaky_domain)
        cfg.sneaky_domain = argv[i];
      else if (!cfg.dest_filepath)
        cfg.dest_filepath = argv[i];
    }
  }
  size_t dom_len = strlen(cfg.sneaky_domain);
  if ((strlen(cfg.dest_filepath) + dom_len) > MAX_DATA_LEN || dom_len > (MAX_DATA_LEN/2))
  {
    printf("Domain name and/or destination filepath is too long\n");
    return cfg;
  }
  cfg.input = input_file(args, argv);
  get_default_dns_server(&cfg);
  return cfg;
}

