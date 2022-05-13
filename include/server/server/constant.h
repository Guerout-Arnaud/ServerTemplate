/*!
** ServerTemplate PROJECT, 2021
** @file server/constant.h
** File description:
** @brief Server constants (macro and const variables) definition
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/

#ifndef SERVER_CONSTANT_H_
    #define SERVER_CONSTANT_H_

    #include "common/constant.h"

    #define MAX_EVENTS 10
    #define MAX_AWAITING_CLIENTS 10

    static const char internal_error_msg[] = "Server internal error. Please try to log again\n";

#endif