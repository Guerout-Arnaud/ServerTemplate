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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <unistd.h>

#include "logger/logger.h"

#include "common/constant.h"
#include "server/struct.h"
#include "server/constant.h"
#include "server/function.h"

#include "game/function.h"

#ifndef ERROR_STR_C
    #define ERROR_STR_C "[" BLINK(BOLD(RED("ERROR"))) "] "
#endif

logger_t *server_logger = NULL;
logger_t *game_logger = NULL;

volatile bool running = false; 

static server_t server_info = {0};
static client_list_t client_list = {0};


static void usage(void)
{
    
}

static void clean_exit(void)
{
    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "Executing clean exit\n"));

    close_server(&server_info, &client_list);
    pthread_mutex_destroy(&client_list.clients_mutex);
    pthread_cond_destroy(&client_list.clients_cond);
    if (server_logger != NULL)
        delete_logger(server_logger);
    if (game_logger != NULL)
        delete_logger(game_logger);
}

int main(int argc, char **argv, char **env)
{
    printf("PID: %d\n", getpid());
    (void) argc;
    (void) argv;
    (void) env;

    atexit(clean_exit);

    pthread_t game_thread = 0;


    pthread_cond_init(&client_list.clients_cond, NULL);
    pthread_mutex_init(&client_list.clients_mutex, NULL);

    // server_logger = create_logger(true, true, NULL, DEBUG);
    server_logger = create_logger(true, false, NULL, DEBUG);

    if (server_logger == NULL) {
        dprintf(STDOUT_FILENO, ERROR_STR_C "Failed to initalize server_logger.\n");
        return (ERROR);
    }

    log_msg(server_logger, LOG_INFO, asprintf(&server_logger->msg, "server_logger started\n"));


    game_logger = create_logger(true, false, NULL, DEBUG);
    if (game_logger == NULL) {
        dprintf(STDOUT_FILENO, ERROR_STR_C "Failed to initalize game_logger.\n");
        return (ERROR);
    }

    log_msg(game_logger, LOG_INFO, asprintf(&game_logger->msg, "game_logger started\n"));


    // ToDo Parse input

    /* ToDo : MOCKUP DATA TO REMOVE */

    server_info.port = 42690;
    if (argc != 1)  {
        server_info.port = atoi(argv[1]);
    }

    /* ToDo : MOCKUP DATA TO REMOVE */

    if (server_init(&server_info, &client_list) == ERROR) {
        log_msg(server_logger, LOG_ERROR, asprintf(&server_logger->msg, "Server initialisation failed\n"));
        return (ERROR);
    }

    if (game_init() == ERROR) {
        log_msg(game_logger, LOG_ERROR, asprintf(&game_logger->msg, "Game initialisation failed\n"));
        return (ERROR);
    }

    running = true;

    pthread_create(&game_thread, NULL, game_loop, &client_list);

    /* ToDo : Thread Game running */
    
    server_loop(&server_info, &client_list);

    pthread_join(game_thread, NULL);

    return (SUCCESS);
}