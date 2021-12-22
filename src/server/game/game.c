/*
** ServerTemplate PROJECT, 2021
** game.c
** File description:
** File containing main game funtions
** Author:
** Arnaud Guerout
** https://github.com/Guerout-Arnaud
** Contributors:
**
*/

#include <unistd.h>
#include <stdio.h>

#include "logger/logger.h"
#include "linked_list/linked_list.h"

#include "common/constant.h"
#include "server/struct.h"

extern logger_t *logger;


int game_init()
{
    /* TODO */
    return (0);
}

void *game_loop(void *users_p)
{
    client_list_t *users = (client_list_t *) users_p;

    log_msg(logger, LOG_INFO, asprintf(&logger->msg, "Game started\n"));
    
    for (EVER) {

        pthread_mutex_lock(&users->clients_mutex);
        if (users->nb_clts_msgs <= 0) {
            log_msg(logger, LOG_INFO, asprintf(&logger->msg, "Waiting for client msg...\n"));
            pthread_cond_wait(&users->clients_cond, &users->clients_mutex);
            log_msg(logger, LOG_INFO, asprintf(&logger->msg, "New message received\n"));
        }
        pthread_mutex_unlock(&users->clients_mutex);
        
        for (int i = 0; i < users->max_connected_clt; i++) {
            if (users->clients[i].in != NULL) {
                log_msg(logger, LOG_INFO, asprintf(&logger->msg, "User %d says %s\n", users->clients[i].socket, users->clients[i].in->content));
                users->clients[i].in = list_del(users->clients[i].in, users->clients[i].in, list);

                pthread_mutex_lock(&users->clients_mutex);
                users->nb_clts_msgs--;
                pthread_mutex_unlock(&users->clients_mutex);
            }
        }
    }
    return (0);
}