/**
 * dns-tunneler
 *
 * @file args_parser.h
 *
 * @brief Header of arguments parsing module.
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
  char *dest_filepath;  // path to the directory
  char *real_path;      // whole path to the file directory+filename
}receiver_config;

receiver_config process_args(int args, char *argv[]);

#endif // DNS_TUNNELER_ARGS_PARSER_H
