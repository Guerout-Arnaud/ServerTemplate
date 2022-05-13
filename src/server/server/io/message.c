/*!
** ServerTemplate PROJECT, 2021
** @file server.c
** File description:
** @brief Server messaging system
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
#include "server/constant.h"
#include "server/struct.h"

extern logger_t *server_logger;

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
    return (msg);
}

int buffer_msg(client_t *client)
{
    message_t *msg = calloc(1, sizeof(*msg));
    
    /* Info : Memory error return success cause message has not been retrived yet */
    if (msg == NULL)
        return (SUCCESS);
        

    /* FixMe : Not splitted around \n*/
    msg->content = receive_msg(client->socket);

    log_msg(server_logger, LOG_DEBUG | LOG_INFO, asprintf(&server_logger->msg, "New message logged : \"%s\".\n", msg->content));

    pthread_mutex_lock(&client->in_mutex);

    client->in = list_add(client->in, msg, list);

    pthread_mutex_unlock(&client->in_mutex);

    return (msg->content != NULL ? SUCCESS : ERROR);
}

void send_msg(int socket, char *msg)
{
    /* ToDo Serialize before send */
    printf("MSG:\"%s\"\n", msg);
    dprintf(socket, "%s%s", msg, MSG_BUFFER_END);
    return;
}

void unbuffer_msg(client_t *client)
{
    for (message_t *msg = client->out; msg != NULL; msg = client->out) {
        send_msg(client->socket, msg->content);

        pthread_mutex_lock(&client->out_mutex);
        client->out = list_del(client->out, msg, list);
        pthread_mutex_unlock(&client->out_mutex);

        free(msg->content);
        free(msg);
    }
}