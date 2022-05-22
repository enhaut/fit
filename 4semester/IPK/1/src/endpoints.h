/**
* IPK project 1
*
* @file endpoints.h
*
* @brief Implementation of endpoints.
*
* @author Samuel Dobroň (xdobro23), FIT BUT
*
*/
#include "response.h"

#ifndef IPK_PROJ1_ENDPOINTS_H
#define IPK_PROJ1_ENDPOINTS_H

#define MAX_ENDPOINT_LEN 9
typedef struct {
    char name[MAX_ENDPOINT_LEN];// endpoint name
    response_t *(*endpoint)();  // ptr to function that handles endpoint
}endpoint_t;
#define ENDPOINTS_COUNT 4

extern endpoint_t ENDPOINTS[ENDPOINTS_COUNT];
int get_endpoint_index(char *buffer);

#endif//IPK_PROJ1_ENDPOINTS_H
