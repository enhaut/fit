//
// Created by Samuel Dobron on 10.02.2022.
//

#ifndef IPK_PROJ1_RESPONSE_H
#define IPK_PROJ1_RESPONSE_H
typedef struct status{
    int code;
    char name[21];
}status_t;

extern status_t statuses[3];

typedef struct {
    status_t status;
    char content[];
}response_t;

response_t * get_response(status_t status, char *content);
#define RESPONSE_HEADER "HTTP/1.1 %d %s\r\nContent-Type: text/plain;\r\n\r\n"
#endif //IPK_PROJ1_RESPONSE_H
