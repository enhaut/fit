/**
* IPK project 1
*
* @file main.c
*
* @brief Implementation of lightweight HTTP server.
*
* @author Samuel Dobroň (xdobro23), FIT BUT
*
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "endpoints.h"
#include "response.h"

#define ERROR_EXIT(message) \
    do{                     \
        perror(message);    \
        exit(EXIT_FAILURE); \
    }while(0)
#define MAX_CONN 5
#define BUFFER_SIZE 128

/**
 * Variables bellow are global to keep them 'free-able'
   by signal handler implemented in another function.
*/
int server_socket;
response_t *response;

/**
 * @brief Signal handler
 *
 */
void sig_handler(int signum)
{
    printf("Received: %d; Shutting down...\n", signum);
    shutdown(server_socket, SHUT_RDWR);

    if (response)
        free(response);
    exit(0);
}

/**
 * @brief Function returns parsed port to listen on.
 * @param args number of arguments
 * @param argv arguments
 * @return port in range <1, 65535>; fails in case of invalid port
 */
int get_port(int args, char **argv)
{
    if (args != 2)
        ERROR_EXIT("Invalid arguments");

    char *end;
    errno = 0;

    long int port = strtol(argv[1], &end, 10);
    // port needs to be in range <1, 65535> and also end needs to be set to the end of argument
    if (port < 1 || port > 65535 || errno == ERANGE || end < (argv[1] + strlen(argv[1])))
        ERROR_EXIT("Invalid port");

    return (uint16_t)port;
}

/**
 * @brief Function sends response to `new_socket`.
 * It uses global variable `response` to get data to send.
 * @param new_socket file descriptor of connection
 */
void send_response(int new_socket)
{
    char response_buffer[256] = {0};  // response could be only 255 characters long, including headers
    sprintf(response_buffer, RESPONSE_HEADER, response->status.code, response->status.name, strlen(response->content));

    printf("Sending response\n");
    send(new_socket, response_buffer, strlen(response_buffer), 0);      // send headers
    send(new_socket, response->content, strlen(response->content), 0);  // send body

    free(response);  // response_t struct is allocated in `get_response()`
    response = NULL;
}

/**
 * @brief Read data from connection. For our usage,
 * only headers contains useful data - endpoint name.
 * @param new_socket file descriptor of connection
 */
void process_connection(int new_socket)
{
    char buffer[BUFFER_SIZE + 1] = {0};
    int readnum = read(new_socket , buffer, BUFFER_SIZE);

    int endpoint = get_endpoint_index(buffer);
    response = (*(ENDPOINTS[endpoint].endpoint))();  // get response
    printf("New request to: %s\n", ENDPOINTS[endpoint].name);

    while (readnum == BUFFER_SIZE)  // read the rest of request
        readnum = read(new_socket, buffer, BUFFER_SIZE);

    send_response(new_socket);

    printf("Closing connection\n");
    close(new_socket);
}

int main(int args, char **argv)
{
    signal(SIGINT, sig_handler);
    int port = get_port(args, argv), new_socket, optval = 1;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        ERROR_EXIT("socket()");

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR , &optval, sizeof(optval)))
        ERROR_EXIT("setsockopt()");

    // Set address to listen in
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
        ERROR_EXIT("bind()");

    if (listen(server_socket, MAX_CONN) < 0)
        ERROR_EXIT("listen()");
    printf("Listening on %d\n", port);

    while (1) // endless loop for accepting connections endlessly
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            fprintf(stderr, "Could not accept connection\n");
            continue;
        }
        process_connection(new_socket);
    }

    return 0;
}
