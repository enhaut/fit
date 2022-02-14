//
// Created by Samuel Dobron on 10.02.2022.
//
#include "response.h"

#ifndef IPK_PROJ1_ENDPOINTS_H
#define IPK_PROJ1_ENDPOINTS_H

#define MAX_ENDPOINT_LEN 9
typedef struct {
    char name[MAX_ENDPOINT_LEN];
    response_t *(*endpoint)();
}endpoint_t;
#define ENDPOINTS_COUNT 4

extern endpoint_t ENDPOINTS[ENDPOINTS_COUNT];
int get_endpoint_index(char *buffer);

#endif//IPK_PROJ1_ENDPOINTS_H
