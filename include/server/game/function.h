/*!
** ServerTemplate PROJECT, 2021
** @file server/function.h
** File description:
** @brief Server main functions prototypes
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/


#ifndef GAME_FUNCTION_H_
    #define GAME_FUNCTION_H_

    #include "server/struct.h"

    int game_init();

    void *game_loop(void *users_p);

    int run_cmd(char *command, user_t *player);


#endif