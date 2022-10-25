/**
 * dns-tunneler
 *
 * @file communication.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#ifndef DNS_TUNNELER_COMMUNICATION_H
#define DNS_TUNNELER_COMMUNICATION_H

#define ERROR_EXIT(msg, ret) do{printf(msg); return ret;}while(0)
int socket_factory(struct sockaddr_in6 *address, int type, int server);
int process_ip(char *ip, struct in6_addr *dest);

#define PREPARE_ADDRESS(name, ip, port)                                                             \
    struct in6_addr raw_addr = IN6ADDR_V4MAPPED_INIT;   /*inet_pton replaces it in case of ipv6*/   \
    int v6 = process_ip((ip), &raw_addr);                                                           \
    struct sockaddr_in6 (name);                                                                     \
    (name).sin6_family = AF_INET6;                                                                  \
    (name).sin6_addr = (raw_addr);                                                                  \
    (name).sin6_port = htons((port))

#define PREPARE_ADDRESS_PTR(addr, ip, port)  \
    (addr)->sin6_family = AF_INET6;    \
    (addr)->sin6_addr = (ip);          \
    (addr)->sin6_port = htons((port))



#endif // DNS_TUNNELER_COMMUNICATION_H
