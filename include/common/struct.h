/*!
** ServerTemplate PROJECT, 2021
** @file common/struct.h
** File description:
** @brief Common structure definition
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/

#ifndef COMMON_STRUCT_H_
    #define COMMON_STRUCT_H_

    #include "linked_list/linked_list.h"

    typedef struct message_s {
        char *content;

        linked_list_t list;
    } message_t;

#endif