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

#include "game/function.h"
#include "game/constant.h"

extern logger_t *logger;

static const char *GAME_COMMANDS[] = {NULL};
static void (*game_cmds[])(char *) = {NULL};

void game_cmd_mngt(char *input, user_t *user)
{
    if (input == NULL)
        return;

    /* ToDo : Make my own strtok_r. which would split on complete string */
    for (char *saveptr = NULL, *command = strtok_r(input, "\n\r", &saveptr);
        command != NULL ;
        command = strtok_r(NULL, "\n\r", &saveptr)) {

        log_msg(logger, LOG_INFO, asprintf(&logger->msg, "New command from Admin : %s\n", command));

        for (size_t i = 0; GAME_COMMANDS[i] != NULL; i++) {
            if (strcmp(command, GAME_COMMANDS[i]) == 0) {
                // printf("%s found !\n", command);
                game_cmds[i](command);
                break;
            }
        }
    }

}