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
#include <signal.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "logger/logger.h"
#include "linked_list/linked_list.h"

#include "common/constant.h"
#include "server/constant.h"
#include "server/struct.h"
#include "server/function.h"


extern volatile bool running;
extern logger_t *server_logger;
extern int server_epollfd;

void close_server(server_t *server_info, client_list_t *clients_list)
{
    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Closing Server.\n"));


    if (server_info->server_socket != -1)
        close(server_info->server_socket);
    if (server_info->signal_fd != -1)
        close(server_info->signal_fd);

    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Disconnecting clients.\n"));
    for (int i = 5; i < clients_list->max_connected_clt; i++) {
        if (clients_list->clients[i].socket <= 0)
            continue;
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
    if (clients_list->clients != NULL)
        free(clients_list->clients);
    if (server_info->epollfd != -1)
        close(server_info->epollfd);
    if (server_epollfd != -1)
        close(server_epollfd);
    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Server Closed.\n"));

}

int server_init(server_t *server_info, client_list_t *clients_list)
{
    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Initialising server.\n"));

    struct epoll_event ev = {0}; 
    struct sockaddr_in socket_in = {0};
    struct protoent *protocol = getprotobyname(PROTOCOL);

    ev.events = EPOLLIN;
    socket_in.sin_family = htonl(AF_INET);
    socket_in.sin_port = htons(server_info->port);
    socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
    
    server_info->signal_fd = setup_signals();
    printf("Signal FD = %d\n", server_info->signal_fd);
    
    server_info->server_socket = socket(AF_INET, SOCK_STREAM, protocol->p_proto);
    server_epollfd = epoll_create(1);
    server_info->epollfd = epoll_create1(0);

    if (server_info->server_socket == -1 || 
        server_info->epollfd == -1 || 
        bind(server_info->server_socket, (const struct sockaddr *)&(socket_in), sizeof(socket_in)) == -1 ||
        listen(server_info->server_socket, MAX_AWAITING_CLIENTS) == -1 ) {
        close_server(server_info, clients_list);
        return (ERROR);
    }

    ev.data.fd = server_info->server_socket;
    if (epoll_ctl(server_epollfd, EPOLL_CTL_ADD, server_info->server_socket, &ev) == -1 ||
        epoll_ctl(server_info->epollfd, EPOLL_CTL_ADD, server_info->server_socket, &ev) == -1) {
        close_server(server_info, clients_list);
        return (ERROR);
    }

    ev.data.fd = server_info->signal_fd;
    if (epoll_ctl(server_info->epollfd, EPOLL_CTL_ADD, server_info->signal_fd, &ev ) == -1) {
        close_server(server_info, clients_list);
        return (ERROR);
    }

    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(server_info->epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {

        close_server(server_info, clients_list);
        // log_msg(server_logger, LOG_DEBUG | LOG_ERROR, asprintf(&server_logger->msg, "Unable to create server\n"));
        return (ERROR);
    }

    
    /*  Hack
        Here the value 5 is not a magic number, it stands for STDIN (0), STDOUT (1), STDERR (2), SignalFd (3) and ServerSocket (4)
        Since I use the file descriptor (socket fd) as an ID to have a more efficient client research
        I needed to add 5 to the default list size even if they wont ever be used. Memory loss can be ignored here  
    */
    clients_list->clients = calloc((size_t)MAX_AWAITING_CLIENTS + (size_t)MAX_EVENTS + 5, sizeof(*clients_list->clients));
    if (clients_list->clients == NULL) {
        close_server(server_info, clients_list);
        log_msg(server_logger, LOG_DEBUG | LOG_ERROR, asprintf(&server_logger->msg, "Calloc failed\n"));
        return (ERROR);
    }

    /* Hack : Do not put this line before calloc is done. Value of max_connected_clt is used to close server*/
    clients_list->max_connected_clt = (size_t)MAX_AWAITING_CLIENTS + (size_t)MAX_EVENTS + 5;
    
    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Server is ready.\n"));

    return (SUCCESS);
}

int server_loop(server_t *server_info, client_list_t *clients_list)
{

    /* FixMe : Use server_logger */
    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Starting Server loop.\n"));

    static unsigned int wait_err = 0;
    int nfds = 0;
    struct epoll_event events[MAX_EVENTS] = {0};

    log_msg(server_logger, LOG_DEBUG | LOG_INFO, asprintf(&server_logger->msg, "Server socket: %d.\n", server_info->server_socket));

    for (;running;) {

        // log_msg(server_logger, LOG_DEBUG | LOG_INFO, asprintf(&server_logger->msg, "WAIT ON\n"));
        nfds = epoll_wait(server_info->epollfd, events, (int)MAX_EVENTS, (int)TIMEOUT);
        // log_msg(server_logger, LOG_DEBUG | LOG_INFO, asprintf(&server_logger->msg, "WAIT OFF\n"));

        if (nfds == -1) {
            log_msg(server_logger, LOG_WARN, asprintf(&server_logger->msg, "Epoll wait failed.\n"));
            sleep(60); /* Hack This prevents for a log spam which could be lead to bigger issues */
            if (wait_err++ > 5) {
                close_server(server_info, clients_list);
                return (ERROR);
            } else
                continue;
        }

        wait_err = 0;

        for (int i = 0; i < nfds; i++) {
            // log_msg(server_logger, LOG_DEBUG | LOG_INFO, asprintf(&server_logger->msg, "Data[%d] on fd: %d.\n", i, events[i].data.fd));
            if (events[i].data.fd == server_info->server_socket) {
                connect_clients(server_info->server_socket, server_info->epollfd, clients_list);
            } else if (events[i].data.fd == server_info->signal_fd) {
                printf("Signal received\n");

                struct signalfd_siginfo rcv_signal = {0};

                if (read(server_info->signal_fd, &rcv_signal, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo))
                    log_msg(server_logger, LOG_WARN, asprintf(&server_logger->msg, "Cannot receive signal. You may have to kill process using : kill %d\n", getpid()));

                if (rcv_signal.ssi_signo == SIGINT || rcv_signal.ssi_signo == SIGQUIT) {
                    running = false;
                    pthread_cond_signal(&clients_list->clients_cond);
                    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Sever shutdown asked by admin using signal\n"));
                    break;
                }

            } else if (events[i].data.fd == STDIN_FILENO) {

                /* ToDo : new process for admin management */
                admin_cmd_mngt();

            } else {
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                    log_msg(server_logger, LOG_DEBUG | LOG_INFO, asprintf(&server_logger->msg, "ERROR or HUP\n"));

                    disconnect_client(server_info->epollfd ,events[i].data.fd, clients_list);

                    events[i].events = 0;
                    continue;
                } 
                if (events[i].events & EPOLLIN) {
                    if (buffer_msg(&clients_list->clients[events[i].data.fd]) == ERROR) {
                        disconnect_client(server_info->epollfd, events[i].data.fd, clients_list);
                        continue;
                    }

                    mod_poll_ev(server_info->epollfd, events[i].data.fd, EPOLLOUT | EPOLLIN);

                    pthread_mutex_lock(&clients_list->clients_mutex);
                    clients_list->nb_clts_msgs++;
                    pthread_mutex_unlock(&clients_list->clients_mutex);
                    pthread_cond_signal(&clients_list->clients_cond);
                }
                if (events[i].events & EPOLLOUT) {
                    if (clients_list->clients[events[i].data.fd].out != NULL) {
                        mod_poll_ev(server_info->epollfd, events[i].data.fd, EPOLLIN);
                        unbuffer_msg(&clients_list->clients[events[i].data.fd]);
                    }
                }
            }
        }
    }
   return (SUCCESS);
}