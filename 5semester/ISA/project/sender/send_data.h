// send_data.h
//
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 6.11.2022

#include <strings.h>

#ifndef DNS_TUNNELER_SEND_DATA_H
#define DNS_TUNNELER_SEND_DATA_H

int send_data(int sock, char *data, size_t len, char *domain, int *last_id, char *path, char *raw_ip);


#endif // DNS_TUNNELER_SEND_DATA_H
