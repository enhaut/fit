/**
* IPK project 1
*
* @file endpoints.c
*
* @brief Implementation of endpoints.
*
* @author Samuel Dobroň (xdobro23), FIT BUT
*
*/

#define  _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "endpoints.h"
#include "response.h"
#include "measure_load.h"

/**
 * @brief Get hostname endpoint
 * @return ptr to response_t struct that contains hostname of machine
 */
response_t * get_hostname()
{
    char hostname[1024] = {0};
    gethostname(hostname, 1023);

    return get_response(statuses[0], hostname);
}

/**
 * @brief CPU name endpoint
 * @return ptr to response_t struct that contains CPU name
 */
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
            if (!strstr(line, "model name"))  // reading lines until "model name" found
                continue;
            response = strstr(line, ":") + 2;  // move ptr behind "model name : "
            break;
        }
        response[strlen(response) - 1] = 0;  // remove trailing \n
        fclose(fp);
    }else
        response = "UNKNOWN";

    response_t *res = get_response(statuses[0], response);
    if (line)
        free(line);  // `getline()` allocates buffer, it needs to be freed
    return res;
}

/**
 * @brief CPU usage endpoing
 * @return ptr to response_t struct that contains CPU usage
 */
response_t * load()
{
    int load = calculate_load();
    if (load == -1)
        return get_response(statuses[2], NULL);  // measurement went wrong, return 500 internal err

    char actual_load[5];
    sprintf(actual_load, "%d%%", load);

    return get_response(statuses[0], actual_load);
}

/**
 * @brief Endpoint for all the requests to non-supported endpoints
 * @return returns struct 400 BAD REQUEST
 */
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

/**
 * @brief Function returns index of requested endpoint in `ENDPOINTS` array.
 * @param buffer received connection headers/body
 * @return index of requested endpoint in `ENDPOINTS` array, in case no
 * corresponding endpoint found, it returns index of last member of that array
 */
int get_endpoint_index(char *buffer)
{
    printf("Looking up for endpoint\n");
    char *endpoint_end = strstr(buffer, "HTTP/1.1");

    if (strstr(buffer, "GET /")  != buffer ||  // HTTP request needs to start with "GET /"
        strlen(buffer) < strlen("GET / HTTP/1.1") || // invalid length of HTTP request
        !endpoint_end)  // HTTP version missing
        return ENDPOINTS_COUNT - 1; // invalid header of HTTP request

    *endpoint_end = 0; // looking up for endpoint in corresponding part of request body only

    for (int i = 0; i < ENDPOINTS_COUNT; i++)
        if (strstr(buffer, ENDPOINTS[i].name))
            return i;

    printf("Endpoint not found, bad request\n");
    return ENDPOINTS_COUNT - 1;
}
