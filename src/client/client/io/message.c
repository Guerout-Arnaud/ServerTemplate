/*!
** ServerTemplate PROJECT, 2021
** @file message.c
** File description:
** @brief Client messaging system
** @author
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors
**
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logger/logger.h"

#include "common/constant.h"
#include "client/struct.h"

extern logger_t *logger;

char *receive_msg(int fd)
{
    int idx = 0;
    ssize_t bytes = 0;
    char buff[MSG_BUFF_SIZE] = {0};
    char *msg = NULL;
    char *msg_tmp = NULL;

    bytes = read(fd, buff, MSG_BUFF_SIZE);

    while (bytes > 0) {
        msg_tmp = realloc(msg, sizeof(*msg) * (idx + bytes + 1));

        if (msg_tmp == NULL)
            break;

        msg = msg_tmp;
        strncpy(&msg[idx], buff, bytes);
        idx = idx + bytes;
        memset(buff, 0, MSG_BUFF_SIZE);

        if (msg[idx - 1] == '\n')
            break;
        bytes = read(fd, buff, MSG_BUFF_SIZE);
    }
    if (msg != NULL)
        msg[idx - 1] = '\0';
    printf("MSG = %s\n", msg);
    return (msg);
}

int buffer_msg(connection_t *client_info)
{
    message_t *msg = calloc(1, sizeof(*msg));

    /* Info : Memory error return success cause message has not been retrived yet */
    if (msg == NULL)
        return (SUCCESS);


    /* FixMe : Not splitted around \n*/
    log_msg(LOG_DEBUG | LOG_INFO, "Trying to get msg from : %d.\n",client_info->client_socket);
    msg->content = receive_msg(client_info->client_socket);

    log_msg(LOG_DEBUG | LOG_INFO, "New message logged : \"%s\".\n", msg->content);

    client_info->in = list_add(client_info->in, msg, list);

    return (msg->content != NULL ? SUCCESS : ERROR);
}

int queue_msg(connection_t *client_info, char *message)
{
    message_t *msg = calloc(1, sizeof(*msg));

    /* Info : Memory error return success cause message has not been retrived yet */
    if (msg == NULL)
        return (SUCCESS);


    /* FixMe : Not splitted around \n*/
    msg->content = message;

    log_msg(LOG_DEBUG | LOG_INFO, "New message logged : \"%s\".\n", msg->content);

    client_info->out = list_add(client_info->out, msg, list);

    return (msg->content != NULL ? SUCCESS : ERROR);
}

void send_msg(int socket, char *msg)
{
    /* ToDo Serialize before send */
    printf("MSG:\"%s\"\n", msg);
    dprintf(socket, "%s%s", msg, MSG_BUFFER_END);
    return;
}

void unbuffer_msg(connection_t *client_info)
{
    for (message_t *msg = client_info->out; msg != NULL; msg = client_info->out) {
        send_msg(client_info->client_socket, msg->content);

        client_info->out = list_del(client_info->out, msg, list);

        free(msg->content);
        free(msg);
    }
}