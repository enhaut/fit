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

#ifndef IN6ADDR_V4MAPPED_INIT
#define IN6ADDR_V4MAPPED_INIT \
   {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
       0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 }}}
#endif

#define PREPARE_ADDRESS(name, ip, port)                                                             \
    struct in6_addr raw_addr = IN6ADDR_V4MAPPED_INIT;   /*inet_pton replaces it in case of ipv6*/   \
    int v6 = process_ip((ip), &raw_addr);                                                           \
    struct sockaddr_in6 (name);                                                                     \
    memset(&(name), 0, sizeof(struct sockaddr_in6));                                                \
    (name).sin6_family = AF_INET6;                                                                  \
    (name).sin6_addr = (raw_addr);                                                                  \
    (name).sin6_port = htons((port))

#define PREPARE_ADDRESS_PTR(addr, ip, port)  \
    (addr)->sin6_family = AF_INET6;    \
    (addr)->sin6_addr = (ip);          \
    (addr)->sin6_port = htons((port))


#define SOCKADDR6_TO_IP(sockaddr) \
    int ipv4 = IN6_IS_ADDR_V4MAPPED(&((sockaddr)->sin6_addr)); \
    struct in_addr ipv4_addr;     \
    struct in6_addr *ipv6_addr;             \
    if (ipv4) \
    {                             \
         memcpy(&ipv4_addr, (sockaddr)->s6_addr[12], sizeof(uint32_t));\
                                  \
    }else                          \
        ipv6_addr = &((sockaddr)->sin6_addr)

#endif // DNS_TUNNELER_COMMUNICATION_H
