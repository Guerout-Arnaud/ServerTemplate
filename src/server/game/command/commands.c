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

extern logger_t *logger;

unsigned int connection_cmd(char *command, user_t *player);


const char *GAME_COMMANDS[] = {"test", /* "start_simulation", "create_world", */ NULL};
unsigned int (*game_cmds[])(char *, user_t *) = {connection_cmd, /* start_simulation, create_world */ NULL};

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
        // printf("%d\n", j);
        // printf("%s\n", GAME_COMMANDS[j]);
        if (strcmp(command, GAME_COMMANDS[j]) == 0) {
            log_msg(LOG_INFO, GREEN("[GAME]") "User %s used command %s\n", player->username, command);
            return (game_cmds[j](command, player));
            break;
        }
    }

    return (ERROR);
}

unsigned int connection_cmd(char *command, user_t *player)
{

    /* FixMe extract teamname from command */
    char *teamname = command;

    team_t *team = get_team(teamname);

    if (teams == NULL) {
        team = malloc(sizeof(*team));
        list_init(team, list);
        team->name = strdup(teamname);
        team->power = 0;
        team->players = malloc(sizeof(*team->players));
        team->players[0] = NULL;
        teams = list_add(teams, team, list);
    }

    int nb_players = 0;
    for (nb_players = 0; team->players[nb_players] != NULL; nb_players++);

    user_t **players_tmp = realloc(team->players, sizeof(*players_tmp) * (nb_players + 2));
    if (players_tmp == NULL) {
        /* ToDo : Error management */
        return (0);
    }
    team->players =  players_tmp;
    team->players[nb_players] = player;
    team->players[nb_players + 1] = NULL;

    return (220);
    // logger_log(logger, LOG_INFO, "con nection command received");
}