/**
 * ipk2
 *
 * @file args_parser.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "stdio.h"
#include "limits.h"

#ifndef IPK2_ARGS_PARSER_H
#define IPK2_ARGS_PARSER_H

#define ERROR_EXIT(message)           \
    do{                               \
        fprintf(stderr, message);     \
        exit(EXIT_FAILURE);           \
    }while(0)


typedef struct {
  char *inter;
  int port;
  int L4;
  int L3;
  unsigned long long to_sniff;
}sniffer_options_t;

#define L3_RANGE 1000
#define ICMP_BIT (1+L3_RANGE)
#define ARP_BIT (2+L3_RANGE)

#define L4_RANGE 2000
#define TCP_BIT (1+L4_RANGE)
#define UDP_BIT (2+L4_RANGE)

#define MAX_PORT 65535

extern sniffer_options_t *snifferOptions;
sniffer_options_t *process_args(int argc, char *argv[]);

#endif // IPK2_ARGS_PARSER_H
