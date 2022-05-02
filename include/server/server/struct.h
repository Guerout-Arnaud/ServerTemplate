/*!
** ServerTemplate PROJECT, 2021
** @file server/struct.h
** File description:
** @brief Server structure definition
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/

#ifndef SERVER_STRUCT_H_
    #define SERVER_STRUCT_H_

    #include <stdbool.h>
    #include <pthread.h>

    #include "linked_list/linked_list.h"
    
    #include "game/struct.h"

    typedef struct message_s {
        char *content;

        linked_list_t list;
    } message_t;

    typedef struct client_s {
        int socket;

        message_t *in;
        message_t *out;

        pthread_mutex_t in_mutex;
        pthread_mutex_t out_mutex;
    
        user_t user_info;
    } client_t;

    typedef struct clients_infos_s {
        int max_connected_clt;
        int nb_clts_msgs;

        pthread_mutex_t clients_mutex;
        pthread_cond_t clients_cond;

        client_t *clients;
    } client_list_t;

    typedef struct server_s
    {
        int port;
        int server_socket;
        int signal_fd;
        int epollfd;
    } server_t;

#endif