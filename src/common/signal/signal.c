/*!
** ServerTemplate PROJECT, 2021
** @file server.c
** File description:
** @brief Server messaging system
** @author 
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors 
**  
*/

#include <stddef.h>
#include <signal.h>
#include <sys/signalfd.h>

#include "common/constant.h"


int setup_signals(void)
{
    int sfd = -1;
    sigset_t mask = {0};

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        return (ERROR);
    } 
    sfd = signalfd(-1, &mask, 0);
    return (sfd);
}