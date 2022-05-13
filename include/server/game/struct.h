/*!
** ServerTemplate PROJECT, 2021
** @file game/struct.h
** File description:
** @brief Game structure definition
** @author 
** [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/

#ifndef GAME_STRUCT_H_
    #define GAME_STRUCT_H_

    #include "linked_list/linked_list.h"

    typedef enum role_e {
        OPERATOR,
        PLAYER
    } role_t;

    typedef struct team_s {
        char *name;
        int power;
        struct user_s **players;

        linked_list_t list;
    } team_t;

    typedef struct user_s {
        role_t role;

        char *username;
        
        team_t *team;
        

    } user_t;

#endif