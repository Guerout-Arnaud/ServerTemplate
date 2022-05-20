/*!
** PetitBain PROJECT, 2022
** @file operator.c
** File description:
** @brief Operator commands management
** @author
** [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** https://github.com/Guerout-Arnaud
** @authors
**
*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "logger/logger.h"

#include <common/constant.h>

#include "game/function.h"
#include "game/struct.h"

unsigned int connection_cmd(char *command, user_t *player);


const char *GAME_COMMANDS[] = {"test", NULL};
unsigned int (*game_cmds[])(char *, user_t *) = {test_cmd, /* start_simulation, create_world */ NULL};

team_t *teams = NULL;

team_t *get_team(char *teamname)
{
    for (team_t *team = teams; team != NULL; team = list_next(team, list)) {
        if (strcmp(team->name, teamname) == 0)
            return (team);
    }
    return (NULL);
}

int run_cmd(char *command, user_t *player)
{
    for(int j = 0; GAME_COMMANDS[j] != NULL; j++) {
        if (strcmp(command, GAME_COMMANDS[j]) == 0) {
            log_msg(LOG_NONE, "[" GREEN(BOLD("GAME")) "] " "User %s used command %s\n", player->username, command);
            return (game_cmds[j](command, player));
            break;
        }
    }

    return (ERROR);
}

unsigned int test_cmd(char *command, user_t *player)
{
    (void) command;
    (void) player;

    log_msg(LOG_NONE, "[" GREEN(BOLD("GAME")) "] " "Command \"test\" success\n");
    return (220);
}