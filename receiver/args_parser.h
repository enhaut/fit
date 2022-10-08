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

#define DOMAIN_MAX_LEN 255
#define DEST_FILEPATH_MAX_LEN 4095

typedef struct {
  char *sneaky_domain;
  char *dest_filepath;  // TODO: use + check whether PATH_MAX is same on both ends
}receiver_config;

receiver_config process_args(int args, char *argv[]);

#endif // DNS_TUNNELER_ARGS_PARSER_H
