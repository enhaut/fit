#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include "errno.h"


#define ERROR_EXIT(message) \
    do{                     \
        perror(message);    \
        exit(EXIT_FAILURE); \
    }while(0)
#define PORT 10000
#define MAX_CONN 5

int server_socket;

void sig_handler(int signum)
{
    fprintf(stderr, "Shutting down...\n");
    shutdown(server_socket, 2);
    exit(0);
}

int get_port(int args, char **argv)
{
    if (args != 2)
        ERROR_EXIT("Invalid arguments");

    char *end;
    errno = 0;

    long int port = strtol(argv[1], &end, 10);
    if (port < 0 || port > 65535 || errno == ERANGE || end < (argv[1] + strlen(argv[1])))
        ERROR_EXIT("Invalid port");

    return (uint16_t)port;
}

int main(int args, char **argv)
{
    signal(SIGINT, sig_handler);
    int port = get_port(args, argv);

    int new_socket;
    size_t valread;
    int optval = 1;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        ERROR_EXIT("socket()");

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR , &optval, sizeof(optval)))
        ERROR_EXIT("setsockopt()");

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address))<0)
        ERROR_EXIT("bind()");

    if (listen(server_socket, MAX_CONN) < 0)
        ERROR_EXIT("listen()");
    printf("Listening on %d\n", port);

    char buffer[1024] = {0};
    char *hello = "Hello from server";
    while (1)
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            fprintf(stderr, "Could not accept connection");
            continue;
        }

        valread = read( new_socket , buffer, 1024);
        printf("%s\n",buffer );
        send(new_socket , hello , strlen(hello) , 0 );
        printf("Hello message sent\n");
        close(new_socket);
    }

    return 0;
}
