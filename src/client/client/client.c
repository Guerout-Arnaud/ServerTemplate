#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <string.h>

#include <unistd.h>

#include "logger/logger.h"

#include "common/constant.h"
#include "common/function.h"
#include "client/struct.h"

extern volatile bool running;
extern logger_t *logger;

#define EPOLL_FDS 4

void mod_poll_ev(int epollfd, int client_socket, uint32_t io)
{
    // struct epoll_event clt_ev = {.events = io | EPOLLET, .data.fd = client_socket};
    struct epoll_event clt_ev = {.events = io, .data.fd = client_socket};

    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, client_socket, &clt_ev) == -1)
    {
        log_msg(LOG_WARN, "Unable to modify poll event.\n");
    }
}

void client_close(connection_t *client_info)
{
    if (client_info->client_socket != -1)
        close(client_info->client_socket);
    if (client_info->signal_fd != -1)
        close(client_info->signal_fd);
    if (client_info->epoll_fd != -1)
        close(client_info->epoll_fd);

    /* ToDo : Free Messages */
}

int client_init(connection_t *client_info)
{
    struct epoll_event ev = {.events = EPOLLIN};
    struct sockaddr_in servaddr = {0};
    struct protoent *protocol = getprotobyname(PROTOCOL);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(client_info->port);
    servaddr.sin_addr.s_addr = inet_addr(client_info->ip_addr);

    client_info->signal_fd = setup_signals();
    printf("Signal FD = %d\n", client_info->signal_fd);

    client_info->client_socket = socket(AF_INET, SOCK_STREAM, protocol->p_proto);
    printf("Socket FD = %d\n", client_info->client_socket);

    client_info->epoll_fd = epoll_create(EPOLL_FDS);
    printf("Epoll FD = %d\n", client_info->epoll_fd);

    if (client_info->client_socket == -1 ||
        client_info->epoll_fd == -1 ||
        connect(client_info->client_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("Failed to connect to server\n");
        client_close(client_info);
        return (ERROR);
    }

    log_msg(LOG_INFO, "Connected to %s:%d\n", client_info->ip_addr, client_info->port);

    ev.data.fd = client_info->signal_fd;
    if (epoll_ctl(client_info->epoll_fd, EPOLL_CTL_ADD, client_info->signal_fd, &ev) == -1)
    {
        client_close(client_info);
        return (ERROR);
    }

    /* ToDo : Replace by pipe stdin */
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(client_info->epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
    {
        client_close(client_info);
        return (ERROR);
    }

    ev.data.fd = client_info->pipe[0];
    if (epoll_ctl(client_info->epoll_fd, EPOLL_CTL_ADD, client_info->pipe[0], &ev) == -1)
    {
        client_close(client_info);
        return (ERROR);
    }

    ev.data.fd = client_info->client_socket;
    if (epoll_ctl(client_info->epoll_fd, EPOLL_CTL_ADD, client_info->client_socket, &ev) == -1)
    {
        client_close(client_info);
        return (ERROR);
    }

    log_msg(LOG_INFO, "Epoll passed.\n");

    return (SUCCESS);
}

int client_loop(connection_t *client_info)
{
    static unsigned int wait_err = 0;
    int nfds = 0;
    struct epoll_event events[EPOLL_FDS] = {0};

    for (; running;)
    {

        nfds = epoll_wait(client_info->epoll_fd, events, EPOLL_FDS, (int)TIMEOUT);

        if (nfds == -1)
        {
            log_msg(LOG_WARN, "Epoll wait failed.\n");
            sleep(60); /* Hack This prevents for a log spam which could be lead to bigger issues */
            if (wait_err++ > 5)
            {
                client_close(client_info);
                return (ERROR);
            }
            else
            {
                continue;
            }
        }

        wait_err = 0;

        for (int i = 0; i < nfds; i++)
        {
            log_msg(LOG_DEBUG | LOG_INFO, "Data[%d] on fd: %d.\n", i, events[i].data.fd);

            if (events[i].data.fd == client_info->signal_fd)
            {
                printf("Signal received\n");

                struct signalfd_siginfo rcv_signal = {0};

                if (read(client_info->signal_fd, &rcv_signal, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo))
                    log_msg(LOG_WARN, "Cannot receive signal. You may have to kill process using : kill %d\n", getpid());

                if (rcv_signal.ssi_signo == SIGINT || rcv_signal.ssi_signo == SIGQUIT)
                {
                    running = false;
                    // pthread_cond_signal(&clients_list->clients_cond);
                    log_msg(LOG_INFO, "Client shutdown asked through signal\n");
                    break;
                }
            }
            /* ToDo : THREAD THIS */
            else if (events[i].data.fd == STDIN_FILENO)
            {
                /* ToDo : read fd 0 + queue */
                char *msg = receive_msg(STDIN_FILENO);
                if (msg == NULL)
                    continue;
                msg[strlen(msg)] = '\n';
                write(client_info->pipe[1], msg, strlen(msg));
                free(msg);
                printf("Message sent\n");
                // queue_msg(client_info, msg);
                // mod_poll_ev(client_info->epoll_fd, client_info->client_socket, EPOLLOUT);
            }
            else if (events[i].data.fd == client_info->pipe[0])
            {

                /* ToDo : read fd 0 + queue */
                char *msg = receive_msg(client_info->pipe[0]);
                if (msg == NULL)
                    continue;
                printf("Message received\n");
                // write(client_info->pipe[1], msg, strlen(msg));
                queue_msg(client_info, msg);
                mod_poll_ev(client_info->epoll_fd, client_info->client_socket, EPOLLOUT);
            }
            else
            {
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
                {
                    log_msg(LOG_ERROR, "Server disconnected: %s\n", (events[i].events & EPOLLERR) ? "EPOLLERR" : "EPOLLHUP");
                    client_close(client_info);
                    return (ERROR);
                }
                if (events[i].events & EPOLLIN)
                {
                    // log_msg(LOG_DEBUG | LOG_INFO, "IN\n");
                    // events[i].events = events[i].events & ~EPOLLIN;

                    if (buffer_msg(client_info) == ERROR)
                    {
                        // disconnect_client(server_info->epollfd, events[i].data.fd, clients_list);
                        continue;
                    }

                    for (message_t *msg = client_info->in; msg != NULL; msg = client_info->in)
                    {
                        log_msg(LOG_INFO, "%s\n", msg->content);
                        client_info->in = list_del(client_info->out, msg, list);

                        free(msg->content);
                        free(msg);
                    }

                    // pthread_mutex_lock(&clients_list->clients_mutex);
                    // clients_list->nb_clts_msgs++;
                    // pthread_mutex_unlock(&clients_list->clients_mutex);
                    // pthread_cond_signal(&clients_list->clients_cond);

                    // log_msg(LOG_DEBUG | LOG_INFO, "END_IN-------\n");
                }
                if (events[i].events & EPOLLOUT)
                {
                    // log_msg(LOG_DEBUG | LOG_INFO, "OUT\n");
                    // events[i].events = events[i].events & ~EPOLLOUT;

                    // char *buffer_msg = NULL;

                    if (client_info->out != NULL)
                    {
                        mod_poll_ev(client_info->epoll_fd, client_info->client_socket, EPOLLIN);
                        unbuffer_msg(client_info);
                    }
                }
            }
        }
    }
    return (SUCCESS);
}