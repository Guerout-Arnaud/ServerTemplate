#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <unistd.h>

#include "logger/logger.h"

#include "common/constant.h"
#include "common/function.h"
#include "client/struct.h"
#include "client/function.h"

extern volatile bool running;

#define EPOLL_FDS 4

void mod_poll_ev(int epollfd, int client_socket, uint32_t io)
{
    struct epoll_event clt_ev = {.events = io, .data.fd = client_socket};

    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, client_socket, &clt_ev) == -1)
    {
        log_msg(LOG_WARN, "Unable to modify poll event.\n");
    }
}

void client_close(connection_t *client_info)
{
    running = false;
    if (client_info->client_socket != -1)
        close(client_info->client_socket);
    if (client_info->signal_fd != -1)
        close(client_info->signal_fd);
    if (client_info->epoll_fd != -1)
        close(client_info->epoll_fd);

    pthread_mutex_lock(&client_info->in_mutex);
    for (message_t *msg = client_info->in; msg != NULL; msg = client_info->in)
    {
        client_info->in = list_del(client_info->in, msg, list);

        free(msg->content);
        free(msg);
    }
    pthread_mutex_unlock(&client_info->in_mutex);
    pthread_mutex_destroy(&client_info->in_mutex);

    pthread_mutex_lock(&client_info->out_mutex);
    for (message_t *msg = client_info->out; msg != NULL; msg = client_info->out)
    {
        client_info->out = list_del(client_info->out, msg, list);

        free(msg->content);
        free(msg);
    }
    pthread_mutex_unlock(&client_info->out_mutex);
    pthread_mutex_destroy(&client_info->out_mutex);
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

    for (; running == true;)
    {

        log_msg( LOG_DEBUG | LOG_INFO, "Waiting for events...\n");
        nfds = epoll_wait(client_info->epoll_fd, events, EPOLL_FDS, (int)TIMEOUT);
        log_msg( LOG_DEBUG | LOG_INFO, "Events received.\n");

        if (nfds == -1)
        {
            log_msg(LOG_WARN, "Epoll wait failed.\n");
            sleep(60); /* Hack This prevents for a log spam which could be lead to bigger issues */
            if (wait_err++ > 5)
            {
                // client_close(client_info);
                log_msg(ERROR, "Too many errors.\n");
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
                struct signalfd_siginfo rcv_signal = {0};

                if (read(client_info->signal_fd, &rcv_signal, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo))
                    log_msg(LOG_WARN, "Cannot receive signal. You may have to kill process using : kill %d\n", getpid());

                if (rcv_signal.ssi_signo == SIGINT || rcv_signal.ssi_signo == SIGQUIT)
                {
                    running = false;
                    log_msg(LOG_INFO, "Client shutdown asked through signal\n");
                    break;
                }
            }
            else if (events[i].data.fd == client_info->pipe[0])
            {
                client_info->out = buffer_msg(client_info->out, &client_info->out_mutex, client_info->pipe[0]);
                mod_poll_ev(client_info->epoll_fd, client_info->client_socket, EPOLLOUT);
            }
            else
            {
                log_msg(LOG_DEBUG | LOG_INFO, "Events on fd=%d are %d\n", events[i].data.fd, events[i].events);
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
                {
                    running = false;
                    log_msg(LOG_INFO, "Server disconnected: %s\n", (events[i].events & EPOLLERR) ? "EPOLLERR" : "EPOLLHUP");
                    dprintf(client_info->client_socket, "%s%s", ACK_MSG, MSG_BUFFER_END);
                    close(client_info->client_socket);
                    client_info->client_socket = -1;
                    break;
                }
                if (events[i].events & EPOLLIN)
                {
                    client_info->in = buffer_msg(client_info->in, &client_info->in_mutex, client_info->client_socket);
                    if (client_info->in == NULL || strncmp(client_info->in->content, SERVER_HUP_CODE, strlen(SERVER_HUP_CODE)) == 0)
                    {
                        running = false;
                        if (client_info->in != NULL) {
                            log_msg(LOG_INFO, "Server asked for disconnection.\n");
                            dprintf(client_info->client_socket, "%s%s", ACK_MSG, MSG_BUFFER_END);
                            close(client_info->client_socket);
                            client_info->client_socket = -1;
                            log_msg(LOG_INFO, "Disconnection accepted.\n");
                        } else {
                            log_msg(LOG_WARN, "Cannot read from server. Server might have closed the connection.\n");
                        }
                        break;
                    }
                }
                if (events[i].events & EPOLLOUT)
                {
                    unbuffer_msg(client_info);
                    mod_poll_ev(client_info->epoll_fd, client_info->client_socket, EPOLLIN);
                }

            }
        }
    }
    log_msg(LOG_INFO, "Client loop ended.\n");
    return (SUCCESS);
}