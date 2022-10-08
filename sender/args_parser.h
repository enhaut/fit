/**
 * dns-tunneler
 *
 * @file args_parser.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#ifndef DNS_TUNNELER_ARGS_PARSER_H
#define DNS_TUNNELER_ARGS_PARSER_H

#include <stdio.h>


typedef struct {
  char ip[46];
  char sneaky_domain[256];
  char dest_filepath[4096];  // TODO: use + check whether PATH_MAX is same on both ends
  FILE *input;
}sender_config;

sender_config process_args(int args, char *argv[]);

#endif // DNS_TUNNELER_ARGS_PARSER_H
