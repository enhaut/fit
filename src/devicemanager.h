/**
 * ipk2
 *
 * @file devicemanager.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "stdbool.h"
#include <pcap/pcap.h>
#include <sys/types.h>

#include "args_parser.h"

#ifndef IPK2_DEVICEMANAGER_H
#define IPK2_DEVICEMANAGER_H

#define ERROR_RETURN(message)             \
      do{                                 \
          fprintf(stderr, message "\n");  \
          return;                         \
      }while(0)

#define DEBUG(fmt, ...) \
  do {                                        \
    fprintf(stderr, fmt "\n", __VA_ARGS__);   \
  } while (0)

/**
 * @brief Macro used to adds BPF rule to `target` string.
 * @param keyword: keyword, that divides BPF commands for example: " and "
 */
#define ADD_RULE(target, keyword, rule, ...)          \
  do {                                                \
    if (pos && target[pos - 1] != '(')                \
      pos += sprintf(target + pos, keyword);          \
    pos += sprintf(target + pos, rule, __VA_ARGS__);  \
  }while(0)

#define MAX_RULES_LENGTH 60  // len("(tcp or udp or icmp or icmp6 or arp or rarp) and port 12345")
#define BPF_OPTIMIZATION 1

extern pcap_if_t * devices_ptr;

pcap_if_t * getDevices();
void print_device(pcap_if_t *device);

#define READING_TIMEOUT 1000
typedef void ( *handler_func_t)(u_char *, const struct pcap_pkthdr*, const u_char *);  // pointer to handler functions
void capture();
#endif // IPK2_DEVICEMANAGER_H
