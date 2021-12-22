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

extern logger_t *logger;

void receive_msg(client_t *client)
{
    int idx = 0;
    ssize_t bytes = 0;
    char buff[MSG_BUFF_SIZE] = {0};
    message_t *msg = calloc(1, sizeof(*msg));
    
    if (msg == NULL)
        return;


    bytes = read(client->socket, buff, MSG_BUFF_SIZE);

    /* FixMe : Not splitted around \n*/
    while (bytes > 0) {
        msg->content = realloc(msg->content, sizeof(*msg->content) * (idx + bytes + 1));
        
        if (msg->content == NULL)
            break;
        strncpy(&msg->content[idx], buff, bytes);
        idx = idx + bytes;
        memset(buff, 0, MSG_BUFF_SIZE);
        

        if (msg->content[idx - 1] == '\n')
            break;
        bytes = read(client->socket, buff, MSG_BUFF_SIZE);
    }
    msg->content[idx - 1] = '\0';
    log_msg(logger, LOG_DEBUG | LOG_INFO, asprintf(&logger->msg, "New message logged : \"%s\".\n", msg->content));

    pthread_mutex_lock(&client->in_mutex);

    client->in = list_add(client->in, msg, list);

    pthread_mutex_unlock(&client->in_mutex);

    return;
}

void send_msg(int socket, char *msg)
{
    /* ToDo Serialize before send */
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