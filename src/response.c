//
// Created by Samuel Dobron on 10.02.2022.
//

#include <stdlib.h>
#include <string.h>

#include "response.h"

status_t statuses[3] = {
        {200, "OK"},
        {400, "BAD REQUEST"},
        {500, "INTERNAL SERVER ERROR"}
};

response_t * get_response(status_t status, char *content)
{
    size_t content_size = 0;
    if (content)
        content_size = strlen(content);

    response_t * response = malloc(sizeof(response_t) + sizeof(char) * (content_size + 1));
    if (!response)
        exit(1);

    if (content)
        strcpy(response->content, content);

    response->content[content_size] = 0;
    response->status = status;

    return response;
}
