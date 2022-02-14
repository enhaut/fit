/**
* IPK project 1
*
* @file response.h
*
* @brief Response related stuff.
*
* @author Samuel Dobroň (xdobro23), FIT BUT
*
*/

#ifndef IPK_PROJ1_RESPONSE_H
#define IPK_PROJ1_RESPONSE_H
typedef struct status{
    int code;       // HTTP status code
    char name[21];  // name of status code
}status_t;

extern status_t statuses[3];
status_t statuses[3] = {
        {200, "OK"},
        {400, "BAD REQUEST"},
        {500, "INTERNAL SERVER ERROR"}
};

typedef struct {
    status_t status;
    char content[]; // ptr to response body
}response_t;

response_t * get_response(status_t status, char *content);
#define RESPONSE_HEADER "HTTP/1.1 %d %s\r\nContent-Type: text/plain;\r\n\r\n"
#endif //IPK_PROJ1_RESPONSE_H
