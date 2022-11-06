/**
 * dns-tunneler
 *
 * @file args_parser.h
 *
 * @brief Header of argument processor module
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#ifndef DNS_TUNNELER_ARGS_PARSER_H
#define DNS_TUNNELER_ARGS_PARSER_H

#include <stdio.h>
#include <arpa/inet.h>


typedef struct {
  char ip[INET6_ADDRSTRLEN];
  char *sneaky_domain;
  char *dest_filepath;
  FILE *input;
}sender_config;

sender_config process_args(int args, char *argv[]);

#endif // DNS_TUNNELER_ARGS_PARSER_H
