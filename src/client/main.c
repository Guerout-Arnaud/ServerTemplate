#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <pthread.h>

#include <stdbool.h>

#include "logger/logger.h"

#include "common/constant.h"

#include "client/struct.h"
#include "client/function.h"

logger_t *logger = NULL;

connection_t client_info = {0};
bool running = false;

static void usage(void)
{

}

static void clean_exit(void)
{
    // log_msg(logger, LOG_INFO, asprintf(&logger->msg, "Executing clean exit\n"));

    client_close(&client_info);
    if (logger != NULL)
        delete_logger(logger);
}

void *ui(void *arg)
{
    // connection_t *client_info = (connection_t *)arg;

    // char buff[2] = {0};
    // while (running) {
    //     read(STDIN_FILENO, &buff, 2);
    //     buff[1] = '\0';
    //     if (buff[0] == 'q') {
    //         running = false;
    //         return (NULL);
    //     }
    //     dprintf(STDOUT_FILENO, "buff: %s\n", buff);
    //     write(client_info, buff, 2);
    // }
    // return (NULL);
}


int main(int argc, char **argv, char **env)
{

    printf("PID: %d\n", getpid());
    (void) argc;
    (void) argv;
    (void) env;

    atexit(clean_exit);

    logger = create_logger(true, false, NULL, DEBUG);
    log_msg(logger, LOG_INFO, asprintf(&logger->msg, "Logger started\n"));

    if (logger == NULL) {
        dprintf(STDOUT_FILENO, ERROR_STR_C "Failed to initalize logger.\n");
        return (ERROR);
    }

    if (argc > 1 && argv[1] != NULL) {
        client_info.ip_addr = argv[1];
    } else {
        client_info.ip_addr = "127.0.0.1";
    }

    if (argc > 2 && argv[1] != NULL) {
        client_info.port = atoi(argv[2]);
    } else {
        client_info.port = 42690;
    }

    if (pipe(client_info.pipe) != 0) {
        log_msg(logger, LOG_ERROR, asprintf(&logger->msg, "Failed to create pipe.\n"));
        return (ERROR);
    }

    log_msg(logger, LOG_INFO, asprintf(&logger->msg, "Connecting to %s:%d\n", client_info.ip_addr, client_info.port));


    if (client_init(&client_info) == ERROR) {
        log_msg(logger, LOG_ERROR, asprintf(&logger->msg, "Failed to initialize client\n"));
        return (ERROR);
    }

    running = true;

    // pthread_create(&game_thread, NULL, ui, &client_info);

    client_loop(&client_info);

    // pthread_join(game_thread, NULL);

    return (SUCCESS);
}