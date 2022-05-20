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
#include "common/function.h"
#include "client/struct.h"


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

message_t *buffer_msg(message_t *msg_list, pthread_mutex_t *mutex, int fd)
{
    char *raw_msg = receive_msg(fd);
    message_t *msg = NULL;

    if (raw_msg == NULL) {
        log_msg(LOG_DEBUG | LOG_WARN, "Received empty message.\n");
        return (msg_list);
    }

    for (char *token = strtok_sub(raw_msg, MSG_BUFFER_END); token != NULL; token = strtok_sub(NULL, MSG_BUFFER_END)) {
        msg = calloc(1, sizeof(*msg));

        /* Info : Memory error return success cause message has not been retrived yet */
        if (msg == NULL) {
            break;
        }

        log_msg(LOG_DEBUG | LOG_INFO, "Trying to get msg from : %d.\n", fd);
        msg->content = strdup(token);

        log_msg(LOG_DEBUG | LOG_INFO, "New message logged : \"%s\".\n", msg->content);

        pthread_mutex_lock(mutex);
        if (strncmp(msg->content, SERVER_HUP_CODE, strlen(SERVER_HUP_CODE)) == 0) {
            log_msg(LOG_DEBUG | LOG_INFO, "HUP message adding message to front.\n");

            list_init(msg, list);
            if (msg_list != NULL) {
                msg->list.next = &msg_list->list;
                msg->list.prev = msg_list->list.prev;
                msg_list->list.prev = &msg->list;
            }
            msg_list = msg;
            pthread_mutex_unlock(mutex);
            break;
        } else {
            msg_list = list_add(msg_list, msg, list);
        }
        pthread_mutex_unlock(mutex);
    }

    free(raw_msg);
    return (msg_list);
}

void send_msg(int socket, char *msg)
{
    /* ToDo Serialize before send */
    dprintf(socket, "%s%s", msg, MSG_BUFFER_END);
    return;
}

void unbuffer_msg(connection_t *client_info)
{
    for (message_t *msg = client_info->out; msg != NULL; msg = client_info->out) {
        send_msg(client_info->client_socket, msg->content);

        pthread_mutex_lock(&client_info->out_mutex);
        client_info->out = list_del(client_info->out, msg, list);
        pthread_mutex_unlock(&client_info->out_mutex);

        free(msg->content);
        free(msg);
    }
}