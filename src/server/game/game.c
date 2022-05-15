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
#include <stdlib.h>
#include <string.h>

#include "logger/logger.h"
#include "linked_list/linked_list.h"

#include "common/constant.h"
#include "server/struct.h"
#include <game/function.h>

extern logger_t *logger;

extern const char **GAME_COMMANDS;
extern unsigned int (**game_cmds)(char *, user_t *);


extern volatile bool running;


int game_init()
{
    /* TODO */
    return (SUCCESS);
}

void *game_loop(void *users_p)
{
    client_list_t *users = (client_list_t *) users_p;

    log_msg(LOG_INFO, GREEN("[GAME]") "Game started\n");

    for (;running;) {

        pthread_mutex_lock(&users->clients_mutex);
        if (users->nb_clts_msgs <= 0) {
            log_msg(LOG_INFO, GREEN("[GAME]") "Waiting for client msg...\n");
            pthread_cond_wait(&users->clients_cond, &users->clients_mutex);
            log_msg(LOG_INFO, GREEN("[GAME]") "New message received\n");
        }
        pthread_mutex_unlock(&users->clients_mutex);

        for (int i = 0; i < users->max_connected_clt; i++) {
            if (users->clients[i].in != NULL) {
                log_msg(LOG_INFO, GREEN("[GAME]") "User %d says %s\n", users->clients[i].socket, users->clients[i].in->content);

                pthread_mutex_lock(&users->clients_mutex);
                message_t *in = users->clients[i].in;
                users->clients[i].in = list_del(users->clients[i].in, users->clients[i].in, list);
                users->nb_clts_msgs--;
                pthread_mutex_unlock(&users->clients_mutex);


                int rc = run_cmd(in->content, &users->clients[i].user_info);

                /* ToDo : Add command management */

                message_t *out = malloc(sizeof(*out));
                list_init(out, list);

                if (rc >= 0) {
                    asprintf(&out->content, "Command %s found. RC = %d.", in->content, rc);
                } else {
                    asprintf(&out->content, "Command %s not found.", in->content);
                }

                free(in->content);
                free(in);

                pthread_mutex_lock(&users->clients_mutex);
                users->clients[i].out = list_add(users->clients[i].out, out, list);
                pthread_mutex_unlock(&users->clients_mutex);

                log_msg(LOG_INFO, GREEN("[GAME]") "Message \"%s\" sent to player.\n", out->content);
            }
        }
    }
    return (0);
}