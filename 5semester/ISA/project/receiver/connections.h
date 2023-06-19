/**
 * dns-tunneler
 *
 * @file connections.h
 *
 * @brief Header of module that implements handling connections.
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#ifndef DNS_TUNNELER_CONNECTIONS_H
#define DNS_TUNNELER_CONNECTIONS_H

#include "args_parser.h"

#define DNS_PORT 53
#define MAX_CONN 1

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

int listen_for_queries(receiver_config *cfg);

#endif // DNS_TUNNELER_CONNECTIONS_H
