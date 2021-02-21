
// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <glib.h>
#include "parse.h"
#include "output.h"
#include "ssl_connection.h"

#define PORT 8080

int main(int argc, char const *argv[])
{
    // int server_fd, new_socket, valread;
    // struct sockaddr_in address;
    // int opt = 1;
    // int addrlen = sizeof(address);
    // char buffer[1024] = {0};

    // if (fork() == 0)
    // {
        StartServer();
        return 0;
    // }

    // // Creating socket file descriptor
    // if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    // {
    //     perror("socket failed");
    //     exit(EXIT_FAILURE);
    // }

    // // Forcefully attaching socket to the port 8080
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
    //                &opt, sizeof(opt)))
    // {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }
    // address.sin_family = AF_INET;
    // address.sin_addr.s_addr = INADDR_ANY;
    // address.sin_port = htons(PORT);

    // // Forcefully attaching socket to the port 8080
    // if (bind(server_fd, (struct sockaddr *)&address,
    //          sizeof(address)) < 0)
    // {
    //     perror("bind failed");
    //     exit(EXIT_FAILURE);
    // }
    // if (listen(server_fd, 3) < 0)
    // {
    //     perror("listen");
    //     exit(EXIT_FAILURE);
    // }
    // while ((new_socket = accept(server_fd, (struct sockaddr *)&address,
    //                             (socklen_t *)&addrlen)) >= 0)
    // {
    //     if (fork() == 0)
    //     {
    //         options_t* options = options_create();
    //         if (TRUE == parse(new_socket, options, NULL))
    //         {
    //             // char *file = options_http_get(options, "page");
    //             // char *type = options_http_get(options, "type");
    //             // char length[10];
    //             // sprintf(length, "%d", output_get_file_length(file));

    //             // output_header(new_socket, type, length, NULL);
    //             // output_file(new_socket, file, NULL);

    //             // options_destroy(options);
    //         }

    //         close(new_socket);
    //         exit(0);
    //     }
    // }

    return 0;
}
