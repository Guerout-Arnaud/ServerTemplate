/*
** ServerTemplate PROJECT, 2021
** main.c
** File description:
** Starting point of the project
** Author:
** Arnaud Guerout
** https://github.com/Guerout-Arnaud
** Contributors:
**
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#ifndef DEBUG 
    #define DEBUG true
#endif

#include "logger/logger.h"


#include "common/constant.h"
#include "server/struct.h"
#include "server/constant.h"
#include "server/function.h"

#include "game/function.h"

#ifndef ERROR_STR_C
    #define ERROR_STR_C "[" BLINK(BOLD(RED("ERROR"))) "] "
#endif

logger_t *logger = NULL;

static void usage(void)
{
    
}

int main(int argc, char **argv, char **env)
{
    printf("PID: %d\n", getpid());
    (void) argc;
    (void) argv;
    (void) env;

    pthread_t game_thread = 0;

    server_t server_info = {0};
    client_list_t client_list = {0};

    pthread_cond_init(&client_list.clients_cond, NULL);
    pthread_mutex_init(&client_list.clients_mutex, NULL);

    // logger = create_logger(true, true, NULL, DEBUG);
    logger = create_logger(true, false, NULL, DEBUG);
    log_msg(logger, LOG_INFO, asprintf(&logger->msg, "Logger started\n"));

    if (logger == NULL) {
        dprintf(STDOUT_FILENO, ERROR_STR_C "Failed to initalize logger.\n");
        return (-1);
    }

    // ToDo Parse input

    /* ToDo : MOCKUP DATA TO REMOVE */

    server_info.port = 42691;
    if (argc != 1)  {
        server_info.port = atoi(argv[1]);
    }


    /* ToDo : MOCKUP DATA TO REMOVE */

    if (server_init(&server_info, &client_list) == ERROR) {
        log_msg(logger, LOG_ERROR, asprintf(&logger->msg, "Server initialisation failed\n"));
        return (ERROR);
    }

    if (game_init() == ERROR) {
        log_msg(logger, LOG_ERROR, asprintf(&logger->msg, "Game initialisation failed\n"));
        // close_server();
        return (ERROR);
    }

    pthread_create(&game_thread, NULL, game_loop, &client_list);

    /* ToDo : Thread Game running */

    server_loop(&server_info, &client_list);

    pthread_join(game_thread, NULL);

    delete_logger(logger);

    pthread_mutex_destroy(&client_list.clients_mutex);
    pthread_cond_destroy(&client_list.clients_cond);

    return (SUCCESS);
}