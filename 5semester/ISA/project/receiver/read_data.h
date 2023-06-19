// read_data.h
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 6.11.2022

#include "dns_receiver_events.h"

#ifndef DNS_TUNNELER_READ_DATA_H
#define DNS_TUNNELER_READ_DATA_H


#define READ(bf, size) do{                          \
    read = recv(sock, bf, size, 0);                 \
    if (read <= 0)                                  \
    {                                               \
        (bf)[0] = '\0';                             \
        if (read == 0)                              \
            return -1;  /* connection closed */     \
        else  /* some kind of error  */             \
            ERROR_EXIT("Could not get data\n", -1); \
    }                                               \
    total += read;                                  \
}while(0)

int receive_dns_packet(int sock, char *buffer);


#endif // DNS_TUNNELER_READ_DATA_H
