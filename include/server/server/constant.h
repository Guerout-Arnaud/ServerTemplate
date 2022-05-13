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

    #ifndef SERVER_FILE_NAME
        #define SERVER_FILE_NAME "server.log"
    #endif

    #define PORT 80
    #define MAX_EVENTS 10
    #define MAX_AWAITING_CLIENTS 10
    #define PROTOCOL "TCP"
    #define MSG_BUFF_SIZE 64
    #define TIMEOUT -1
    // #define TIMEOUT 1000

    static const char MSG_BUFFER_END[] = "\n";
    // static const char MSG_BUFFER_END[] = "\r\n\r\n";

    static const char internal_error_msg[] = "Server internal error. Please try to log again\n";

#endif