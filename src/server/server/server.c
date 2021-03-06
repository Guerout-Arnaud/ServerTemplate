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
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "logger/logger.h"
#include "linked_list/linked_list.h"

#include "common/constant.h"
#include "common/function.h"
#include "server/constant.h"
#include "server/struct.h"
#include "server/function.h"


extern volatile bool running;
extern int server_epollfd;

void close_server(server_t *server_info, client_list_t *clients_list)
{
    log_msg(LOG_INFO, "Closing Server.\n");

    if (server_info->signal_fd != -1)
        close(server_info->signal_fd);

    log_msg(LOG_INFO, "Disconnecting clients.\n");

    int online_clients = 0;

    for (int i = 5; i < clients_list->max_connected_clt; i++) {
        if (clients_list->clients[i].socket <= 0)
            continue;

        online_clients++;

        dprintf(clients_list->clients[i].socket, "%s-%s%s", SERVER_HUP_CODE, internal_error_msg, MSG_BUFFER_END);

        mod_poll_ev(server_info->epollfd, clients_list->clients[i].socket, EPOLLIN);

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

    log_msg(LOG_INFO, "Waiting for Acknoledgment.\n");

    struct epoll_event events[MAX_EVENTS] = {0};
    char buff[MSG_BUFF_SIZE] = {0};

    for (int nfds = 0, wait_limit = 0; online_clients > 0 && wait_limit < 10;) {
        nfds = epoll_wait(server_info->epollfd, events, MAX_EVENTS, 500);
        if (nfds == -1) {
            wait_limit++;
            log_msg(LOG_INFO, "Current limit: %d\n");
        }
        else
            wait_limit = 0;
        for (int i = 0; i < nfds; i++) {
            memset(buff, 0, MSG_BUFF_SIZE);
            read(events[i].data.fd, buff, MSG_BUFF_SIZE);
            if (strncmp(buff, ACK_MSG, strlen(ACK_MSG)) != 0)
                continue;
            msleep(500);
            close(events[i].data.fd);
            clients_list->clients[i].socket = -1;
            online_clients--;
        }
    }

    if (clients_list->clients != NULL)
        free(clients_list->clients);
    if (server_info->epollfd != -1)
        close(server_info->epollfd);
    if (server_epollfd != -1)
        close(server_epollfd);
    if (server_info->server_socket != -1)
        close(server_info->server_socket);
    log_msg(LOG_INFO, "Server Closed.\n");
}

int server_init(server_t *server_info, client_list_t *clients_list)
{
    log_msg(LOG_INFO, "Initialising server.\n");

    struct epoll_event ev = {.events = EPOLLIN};
    struct sockaddr_in socket_in = {0};
    struct protoent *protocol = getprotobyname(PROTOCOL);

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
        log_msg(LOG_DEBUG | LOG_ERROR, "Unable to create server\n");
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
        log_msg(LOG_DEBUG | LOG_ERROR, "Calloc failed\n");
        return (ERROR);
    }

    /* Hack : Do not put this line before calloc is done. Value of max_connected_clt is used to close server*/
    clients_list->max_connected_clt = (size_t)MAX_AWAITING_CLIENTS + (size_t)MAX_EVENTS + 5;

    log_msg(LOG_INFO, "Server is ready.\n");

    return (SUCCESS);
}

int server_loop(server_t *server_info, client_list_t *clients_list)
{

    /* FixMe : Use logger */
    log_msg(LOG_INFO, "Starting Server loop.\n");

    static unsigned int wait_err = 0;
    int nfds = 0;
    struct epoll_event events[MAX_EVENTS] = {0};

    log_msg(LOG_DEBUG | LOG_INFO, "Server socket: %d.\n", server_info->server_socket);

    for (;running;) {

        log_msg(LOG_DEBUG | LOG_INFO, "WAIT ON\n");
        nfds = epoll_wait(server_info->epollfd, events, (int)MAX_EVENTS, (int)TIMEOUT);
        log_msg(LOG_DEBUG | LOG_INFO, "WAIT OFF\n");

        if (nfds == -1) {
            log_msg(LOG_WARN, "Epoll wait failed.\n");
            sleep(60); /* Hack This prevents for a log spam which could be lead to bigger issues */
            if (wait_err++ > 5) {
                log_msg(LOG_ERROR, "Too many epoll fails.\n");
                close_server(server_info, clients_list);
                return (ERROR);
            } else
                continue;
        }

        wait_err = 0;

        for (int i = 0; i < nfds; i++) {
            log_msg(LOG_DEBUG | LOG_INFO, "Data[%d] on fd: %d.\n", i, events[i].data.fd);
            if (events[i].data.fd == server_info->server_socket) {
                connect_clients(server_info->server_socket, server_info->epollfd, clients_list);
            } else if (events[i].data.fd == server_info->signal_fd) {
                struct signalfd_siginfo rcv_signal = {0};

                if (read(server_info->signal_fd, &rcv_signal, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo))
                    log_msg(LOG_WARN, "Cannot receive signal. You may have to kill process using : kill %d\n", getpid());

                if (rcv_signal.ssi_signo == SIGINT || rcv_signal.ssi_signo == SIGQUIT) {
                    running = false;
                    pthread_cond_signal(&clients_list->clients_cond);
                    log_msg(LOG_INFO, "Sever shutdown asked by admin using signal\n");
                    break;
                }

            } else if (events[i].data.fd == STDIN_FILENO) {

                /* ToDo : new process for admin management */
                admin_cmd_mngt();

            } else {
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                    log_msg(LOG_DEBUG | LOG_WARN, "ERROR or HUP\n");

                    disconnect_client(server_info->epollfd ,events[i].data.fd, clients_list);

                    events[i].events = 0;
                    continue;
                }
                if (events[i].events & EPOLLIN) {
                    log_msg(LOG_DEBUG | LOG_INFO, "EPOLLIN\n");

                    if (buffer_msg(&clients_list->clients[events[i].data.fd]) == ERROR) {
                        disconnect_client(server_info->epollfd, events[i].data.fd, clients_list);
                        continue;
                    }

                    mod_poll_ev(server_info->epollfd, events[i].data.fd, EPOLLOUT);

                    pthread_mutex_lock(&clients_list->clients_mutex);
                    clients_list->nb_clts_msgs++;
                    pthread_mutex_unlock(&clients_list->clients_mutex);
                    pthread_cond_signal(&clients_list->clients_cond);

                    log_msg(LOG_DEBUG | LOG_INFO, "END EPOLLIN\n");
                }
                if (events[i].events & EPOLLOUT) {
                    log_msg(LOG_DEBUG | LOG_INFO, "EPOLLOUT\n");
                    // events[i].events = events[i].events & ~EPOLLOUT;

                    if (clients_list->clients[events[i].data.fd].out != NULL) {
                        mod_poll_ev(server_info->epollfd, events[i].data.fd, EPOLLIN);
                        unbuffer_msg(&clients_list->clients[events[i].data.fd]);
                    }
                    log_msg(LOG_DEBUG | LOG_INFO, "END EPOLLOUT\n");
                }
            }
        }
    }
   return (SUCCESS);
}