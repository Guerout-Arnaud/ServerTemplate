/*!
** ServerTemplate PROJECT, 2021
** @file common/constant.h
** File description:
** @brief Constants (macro and const variables) definition common to the entire project
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/


#ifndef COMMON_CONSTANT_H_
    #define COMMON_CONSTANT_H_

    #define EVER ;;

    #ifndef DEBUG 
        #define DEBUG false
    #endif

    #ifndef PORT
        #define PORT 80
    #endif

    #ifndef PROTOCOL
        #define PROTOCOL "TCP"
    #endif
    
    #ifndef MSG_BUFF_SIZE
        #define MSG_BUFF_SIZE 64
    #endif
    
    #ifndef TIMEOUT
        #define TIMEOUT -1
    #endif

    #ifndef ERROR_STR_C
        #define ERROR_STR_C "[" BLINK(BOLD(RED("ERROR"))) "] "
#   endif

    static const int SUCCESS = 0;
    static const int ERROR = -1;

    // static const char MSG_BUFFER_END[] = "\r\n\r\n";
    static const char MSG_BUFFER_END[] = "\n";

#endif
