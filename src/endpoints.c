//
// Created by Samuel Dobron on 10.02.2022.
//
#define  _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "endpoints.h"
#include "response.h"
#include "measure_load.h"

response_t * get_hostname()
{
    char hostname[1024] = {0};
    gethostname(hostname, 1023);

    return get_response(statuses[0], hostname);
}

response_t * cpu_name()
{
    FILE *fp;
    char *line = NULL;
    char *response = NULL;
    size_t len = 0;

    if ((fp = fopen("/proc/cpuinfo", "r")))
    {
        while (getline(&line, &len, fp) != -1)
        {
            if (!strstr(line, "model name"))
                continue;
            response = strstr(line, ":") + 2;  // move ptr behind ": "
            break;
        }
        response[strlen(response) - 1] = 0;  // remove trailing \n
        fclose(fp);
    }else
        response = "UNKNOWN";

    response_t *res = get_response(statuses[0], response);
    if (line)
        free(line);
    return res;
}

response_t * load()
{
    int load = calculate_load();
    if (load == -1)
        return get_response(statuses[2], NULL);

    char actual_load[5];
    sprintf(actual_load, "%d%%", load);

    return get_response(statuses[0], actual_load);
}

response_t * bad_request()
{
    return get_response(statuses[1], NULL);
}

endpoint_t ENDPOINTS[ENDPOINTS_COUNT] = {
        {"/hostname", get_hostname},
        {"/cpu-name", cpu_name},
        {"/load", load},
        {"BAD_REQ", bad_request}  // keep at the last place
};

int get_endpoint_index(char *buffer)
{
    printf("Looking up for endpoint\n");
    char *endpoint_end = strstr(buffer, "HTTP/");
    *endpoint_end = 0; // looking up for endpoint in corresponding part of request body only

    for (int i = 0; i < ENDPOINTS_COUNT; i++)
        if (strstr(buffer, ENDPOINTS[i].name))
            return i;

    printf("Endpoint not found, bad request\n");
    return ENDPOINTS_COUNT - 1;
}
