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


#endif