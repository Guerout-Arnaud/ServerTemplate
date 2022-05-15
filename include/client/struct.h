/*!
** ServerTemplate PROJECT, 2021
** @file client/struct.h
** File description:
** @brief Client structure definition
** @author
** [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors
**
*/

#ifndef CLIENT_STRUCT_H_
    #define CLIENT_STRUCT_H_

    #include "common/struct.h"

    typedef struct connection_s {
        int client_socket;
        int signal_fd;
        int epoll_fd;
        int pipe[2];


        int port;
        char *ip_addr;

        message_t *in;
        message_t *out;

        pthread_mutex_t in_mutex;
        pthread_mutex_t out_mutex;


    } connection_t;

#endif