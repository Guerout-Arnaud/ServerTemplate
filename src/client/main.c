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

connection_t client_info = {0};
bool running = false;

static void usage(void)
{
}

static void clean_exit(void)
{
    // log_msg(LOG_INFO, "Executing clean exit\n");

    client_close(&client_info);
    logger_destroy();
}

int main(int argc, char **argv, char **env)
{

    printf("PID: %d\n", getpid());
    (void)argc;
    (void)argv;
    (void)env;

    atexit(clean_exit);

    pthread_t ui_thread = 0;

    if (logger_init(DEBUG, true, true) == -1)
    {
        dprintf(STDOUT_FILENO, ERROR_STR_C "Failed to initalize logger.\n");
        return (ERROR);
    }
    log_msg(LOG_INFO, "Logger started\n");


    if (argc > 1 && argv[1] != NULL)
    {
        client_info.ip_addr = argv[1];
    }
    else
    {
        client_info.ip_addr = "127.0.0.1";
    }

    if (argc > 2 && argv[1] != NULL)
    {
        client_info.port = atoi(argv[2]);
    }
    else
    {
        client_info.port = 42690;
    }

    log_msg(LOG_INFO, "Connecting to %s:%d\n", client_info.ip_addr, client_info.port);


   if (ui_init(&client_info) == ERROR)
    {
        log_msg(LOG_ERROR, "Failed to initialize UI\n");
        return (ERROR);
    }

    if (client_init(&client_info) == ERROR)
    {
        log_msg(LOG_ERROR, "Failed to initialize client\n");
        return (ERROR);
    }

    running = true;

    pthread_create(&ui_thread, NULL, ui, &client_info);

    client_loop(&client_info);

    pthread_join(ui_thread, NULL);

    return (SUCCESS);
}