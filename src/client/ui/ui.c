#include <stdbool.h>

#include <unistd.h>

#include "logger/logger.h"

#include "common/constant.h"

#include "client/struct.h"
#include "client/function.h"

extern bool running;

int ui_init(connection_t *client_info)
{
    if (pipe(client_info->pipe) != 0)
    {
        log_msg(LOG_ERROR, "Failed to create pipe.\n");
        return (ERROR);
    }

    if (dup2(STDIN_FILENO, client_info->pipe[0]) < 0)
    {
        log_msg(LOG_ERROR, "Failed to redirect stdin.\n");
        return (ERROR);
    }

    return (SUCCESS);
}

void *ui(void *arg)
{
    connection_t *client_info = (connection_t *)arg;

    /* Info : get data and write them on pipe (/!\ wite on pipe[1] not 0, STDIN is a READ ONLY fd) */
    // close(client_info->pipe[0]);
    log_msg(LOG_INFO, "UI thread started\n");
    log_msg(LOG_INFO, "UI thread dup2 done\n");

    while (running)
    {
        if (client_info->in != NULL)
        {
            // log_msg(LOG_INFO, "Mag found\n");
            /* lock mutex */
            pthread_mutex_lock(&client_info->in_mutex);
            for (message_t *msg = client_info->in; msg != NULL; msg = client_info->in)
            {
                log_msg(LOG_INFO,"[" GREEN(BOLD("UI")) "] " "Message : %s\n", msg->content);

                client_info->in = list_del(client_info->in, msg, list);

                free(msg->content);
                free(msg);
            }
            pthread_mutex_unlock(&client_info->in_mutex);
        }
    };
    return (NULL);
}