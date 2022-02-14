//
// Created by Samuel Dobron on 10.02.2022.
//

#include "endpoints.h"
#include "string.h"
#include "response.h"

response_t * get_hostname()
{
    char hostname[] = "merlin.fit.vutbr.cz";
    return get_response(statuses[0], hostname);
}

response_t * cpu_name()
{
    return get_response(statuses[0], "xeon");
}

response_t * load()
{
    return get_response(statuses[0], "420%");
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
    char *endpoint_end = strstr(buffer, "HTTP/");
    *endpoint_end = 0; // looking up for endpoint in corresponding part of request body only

    for (int i = 0; i < ENDPOINTS_COUNT; i++)
        if (strstr(buffer, ENDPOINTS[i].name))
            return i;

    return ENDPOINTS_COUNT - 1;
}
