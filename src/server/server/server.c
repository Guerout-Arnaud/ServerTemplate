/*!
** ServerTemplate PROJECT, 2021
** @file server.c
** File description:
** @brief Server main functions
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/

/* FixMe : Linux Dependant */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
// #include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "logger/logger.h"
#include "linked_list/linked_list.h"

#include "common/constant.h"
#include "server/constant.h"
#include "server/struct.h"
#include "server/function.h"

extern logger_t *logger;
extern int server_epollfd;


int server_init(server_t *server_info, client_list_t *clients_list)
{
    struct epoll_event ev = {0}; 
    struct sockaddr_in socket_in = {0};
    struct protoent *protocol = getprotobyname(PROTOCOL);

    ev.events = EPOLLIN;
    socket_in.sin_family = htonl(AF_INET);
    socket_in.sin_port = htons(server_info->port);
    socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
    
    server_info->server_socket = socket(AF_INET, SOCK_STREAM, protocol->p_proto);
    server_epollfd = epoll_create(1);
    server_info->epollfd = epoll_create1(0);

    ev.data.fd = server_info->server_socket;

    if (server_info->server_socket == -1 || 
        server_info->epollfd == -1 || 
        bind(server_info->server_socket, (const struct sockaddr *)&(socket_in), sizeof(socket_in)) == -1 ||
        listen(server_info->server_socket, MAX_AWAITING_CLIENTS) == -1 ||
        epoll_ctl(server_epollfd, EPOLL_CTL_ADD, server_info->server_socket, &ev) == -1 ||
        epoll_ctl(server_info->epollfd, EPOLL_CTL_ADD, server_info->server_socket, &ev) == -1 ||
        epoll_ctl(server_info->epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {

        if (server_info->server_socket != -1)
            close(server_info->server_socket);

        if (server_epollfd != -1)
            close(server_epollfd);

        if (server_info->epollfd != -1)
            close(server_info->epollfd);

        if (server_info->epollfd != -1)
            close(server_info->epollfd);
        
        log_msg(logger, LOG_DEBUG | LOG_ERROR, asprintf(&logger->msg, "Unable to create server\n"));

        return (ERROR);
    }

    /*  Hack
        Here the value 3 is not a magic number, it stands for STDIN (0), STDOUT (1) and STDERR (2) 
        Since I use the file descriptor (socket fd) as an ID to have a more efficient client research
        I needed to add 3 to the default list size even if 1 and 2 wont ever be used. Memory loss ignored here  
    */
    clients_list->max_connected_clt = (size_t)MAX_AWAITING_CLIENTS + (size_t)MAX_EVENTS + 3;
    
    clients_list->clients = calloc(clients_list->max_connected_clt, sizeof(*clients_list->clients));
    if (clients_list->clients == NULL) {
        log_msg(logger, LOG_DEBUG | LOG_ERROR, asprintf(&logger->msg, "Calloc failed\n"));
        return (ERROR);
    }

    return (SUCCESS);
}

static void close_server(server_t *server_info, client_list_t *clients_list)
{
    close(server_info->server_socket);
    for (int i = 0; i < clients_list->max_connected_clt; i++) {

        dprintf(clients_list->clients[i].socket, "%s%s", internal_error_msg, MSG_BUFFER_END);
        close(clients_list->clients[i].socket);

        for (message_t *next = NULL; clients_list->clients[i].in != NULL; clients_list->clients[i].in = next) {
            next = list_next(clients_list->clients[i].in, list);
            free(clients_list->clients[i].in->content);
            free(clients_list->clients[i].in);
        }

        for (message_t *next = NULL; clients_list->clients[i].out != NULL; clients_list->clients[i].out = next) {
            next = list_next(clients_list->clients[i].out, list);
            free(clients_list->clients[i].out->content);
            free(clients_list->clients[i].out);
        }
    }
    free(clients_list->clients);
    close(server_info->epollfd);
    close(server_epollfd);
}

int server_loop(server_t *server_info, client_list_t *clients_list)
{
    printf("Server Ready\n");

    static unsigned int wait_err = 0;
    int nfds = 0;
    struct epoll_event events[MAX_EVENTS] = {0};

    log_msg(logger, LOG_DEBUG | LOG_INFO, asprintf(&logger->msg, "Server socket: %d.\n", server_info->server_socket));


    for (EVER) {
        nfds = epoll_wait(server_info->epollfd, events, (int)MAX_EVENTS, (int)TIMEOUT);
        if (nfds == -1) {
            log_msg(logger, LOG_WARN, asprintf(&logger->msg, "Epoll wait failed.\n"));
            sleep(60); /* Hack This prevents for a log spam which could be lead to bigger issues */
            if (wait_err++ > 5) {
                close_server(server_info, clients_list);
                return (ERROR);
            } else
                continue;
        }

        wait_err = 0;

        for (int i = 0; i < nfds; i++) {
            log_msg(logger, LOG_DEBUG | LOG_INFO, asprintf(&logger->msg, "Data on fd: %d.\n", events[i].data.fd));
            if (events[i].data.fd == server_info->server_socket) {
                connect_clients(server_info->server_socket, server_info->epollfd, clients_list);
            } else if (events[i].data.fd == STDIN_FILENO) {
                printf("TEEEEEEEEEEEEEEEEEEEEEEEEEEEEST\n");
                log_msg(logger, LOG_INFO, asprintf(&logger->msg, BOLD(BLINK("NEW MESSAGE FROM STDIN\n"))));
            } else {
                if (events[i].events & EPOLLIN) {
                    receive_msg(&clients_list->clients[events[i].data.fd]);
                    pthread_mutex_lock(&clients_list->clients_mutex);
                    clients_list->nb_clts_msgs++;
                    pthread_mutex_unlock(&clients_list->clients_mutex);
                    pthread_cond_signal(&clients_list->clients_cond);
                } else if (events[i].events & EPOLLOUT) {
                    unbuffer_msg(&clients_list->clients[events[i].data.fd]);
                } else if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                    disconnect_client(server_info->epollfd ,events[i].data.fd, clients_list);
                }
            }
        }
    }
   return (SUCCESS);
}