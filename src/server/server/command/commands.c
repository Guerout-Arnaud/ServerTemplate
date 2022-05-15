/*!
** ServerTemplate PROJECT, 2021
** @file commands.c
** File description:
** @brief Admin commands management
** @author
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors
**
*/

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>

#include "logger/logger.h"

#include "server/function.h"
#include "server/constant.h"

extern logger_t *logger;

static const char *ADMIN_COMMANDS[] = {NULL};
static void (*admin_cmds[])(char *) = {NULL};

void admin_cmd_mngt(void)
{
    char *msg_buff = receive_msg(STDIN_FILENO);

    if (msg_buff == NULL)
        return;

    /* ToDo : Make my own strtok_r. which would split on complete string */
    for (char *saveptr = NULL, *command = strtok_r(msg_buff, "\n\r", &saveptr);
        command != NULL ;
        command = strtok_r(NULL, "\n\r", &saveptr)) {

        log_msg(LOG_INFO, "New command from Admin : %s\n", command);

        for (size_t i = 0; ADMIN_COMMANDS[i] != NULL; i++) {
            if (strcmp(command, ADMIN_COMMANDS[i]) == 0) {
                // printf("%s found !\n", command);
                admin_cmds[i](command);
                break;
            }
        }
    }

}