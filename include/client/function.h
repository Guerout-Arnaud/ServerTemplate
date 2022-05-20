/*!
** ServerTemplate PROJECT, 2021
** @file server/function.h
** File description:
** @brief Server main functions prototypes
** @author
**  [Arnaud Guerout](https://github.com/Guerout-Arnaud)
** Contributors:
** @authors
**
*/

#ifndef CLIENT_FUNCTION_H_
    #define CLIENT_FUNCTION_H_

    #include "client/struct.h"

    /**
     * @brief initialisation function
     *
     * @param client_info
     * @return int Status of the client either 0 (Success) or -1 (Error)
     */
    int client_init(connection_t *client_info);

    /**
     * @brief This function is the main client loop that manages user/server inputs/outputs
     *
     * @param client_info
     * @return int Status of the client either 0 (Success) or -1 (Error)
     */
    int client_loop(connection_t *client_info);

    /**
     * @brief This function manages the closing of the client
     *
     * @param client_info
     * @return void
     */
    void client_close(connection_t *client_info);


    char *receive_msg(int fd);
    message_t *buffer_msg(message_t *msg_list, pthread_mutex_t *mutex, int fd);

    // int buffer_msg(connection_t *client_info);
    // int queue_msg(connection_t *client_info, char *message);
    void unbuffer_msg(connection_t *client_info);


    void *ui(void *arg);
    int ui_init(connection_t *client_info);


#endif