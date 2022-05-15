/*!
** ServerTemplate PROJECT, 2021
** @file server.c
** File description:
** @brief Server connetion functions
** @author
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors
**
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/epoll.h>

#include "logger/logger.h"

#include "server/constant.h"
#include "server/struct.h"

int server_epollfd = -1;

void connect_clients(int server_socket, int epollfd, client_list_t *list)
{
    int client_socket = -1;
    int nfds = 1;
    client_t *clients_tmp = NULL;
    struct epoll_event srv_ev = {.events = EPOLLIN};
    struct epoll_event clt_ev = {.events = EPOLLIN};
    // struct epoll_event clt_ev = {.events = EPOLLIN | EPOLLET};
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);

    while (nfds == 1 && srv_ev.events & EPOLLIN) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_socket == -1) {
            log_msg(LOG_WARN, "Error while accepting client.\n");
            return;
        }

        log_msg(LOG_INFO, "Client Socket is %d\n", client_socket);

        if (client_socket > list->max_connected_clt) {
            clients_tmp = realloc(list->clients, list->max_connected_clt + (size_t)MAX_AWAITING_CLIENTS + (size_t)MAX_EVENTS);
            if (clients_tmp != NULL) {
                list->clients = clients_tmp;
                list->max_connected_clt = list->max_connected_clt + (size_t)MAX_AWAITING_CLIENTS + (size_t)MAX_EVENTS;
            } else {
                log_msg(LOG_WARN, "Unable to accept client. Out of memory.\n");
                dprintf(client_socket, "%s%s", internal_error_msg, MSG_BUFFER_END);
                close(client_socket);
            }
        }

        clt_ev.data.fd = client_socket;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_socket, &clt_ev) == -1) {
            log_msg(LOG_WARN, "Unable to add client to poll list.\n");
            dprintf(client_socket, "%s%s", internal_error_msg, MSG_BUFFER_END);
            close(client_socket);
        }

        memset(&list->clients[client_socket], 0, sizeof(*list->clients));
        list->clients[client_socket].socket = client_socket;
        pthread_mutex_init(&list->clients[client_socket].in_mutex, NULL);
        pthread_mutex_init(&list->clients[client_socket].out_mutex, NULL);

        nfds = epoll_wait(server_epollfd, &srv_ev, 1, 0);
    }
}

void disconnect_client(int epollfd, int client_socket, client_list_t *list)
{
    struct epoll_event clt_ev = {.events = EPOLLIN | EPOLLOUT | EPOLLET};

    memset(&list->clients[client_socket], 0, sizeof(*list->clients));
    epoll_ctl(epollfd, EPOLL_CTL_DEL, client_socket, &clt_ev);
    pthread_mutex_destroy(&list->clients[client_socket].in_mutex);
    pthread_mutex_destroy(&list->clients[client_socket].out_mutex);
    close(client_socket);

    log_msg(LOG_INFO, "Client %d disconneted.\n", client_socket);
}

void mod_poll_ev(int epollfd, int client_socket, uint32_t io)
{
    // struct epoll_event clt_ev = {.events = io | EPOLLET, .data.fd = client_socket};
    struct epoll_event clt_ev = {.events = io, .data.fd = client_socket};

    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, client_socket, &clt_ev) == -1) {
        log_msg(LOG_WARN, "Unable to modify poll event.\n");
    }
}