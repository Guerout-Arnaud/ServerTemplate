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

#ifndef SERVER_FUNCTION_H_
    #define SERVER_FUNCTION_H_

    #include "server/struct.h"

    /*! @fn server_init 
    * @brief Server initialisation function.
    * 
    * @param server_info 
    * @return int 
    */
    int server_init(server_t *server_info, client_list_t *clients_list);

    /*! @fn setup_signals 
    * @brief Server initialisation function.
    * 
    * @param void 
    * @return int 
    */
    int setup_signals(void);

    /** @fn server_loop
     * @brief This function is the main server loop that waits for messages
     *  and guide them to correct funtion
     * 
     * @param server_info 
     * @param clients_list 
     * @return int Status of the server either 0 (Success) or -1 (Error)
     */
    int server_loop(server_t *server_info, client_list_t *clients_list);

    /** @fn close_server
     * @brief This function manages the closing of the server
     * 
     * @param server_info 
     * @param clients_list 
     * @return void
     */
    void close_server(server_t *server_info, client_list_t *clients_list);

    /*! @fn connect_clients 
    * @brief New clients management function.
    * 
    * @param server_socket 
    * @param epollfd
    * @param clients_list 
    * @return void 
    */
    void connect_clients(int server_socket, int epollfd, client_list_t *list);

    /*! @fn disconnect_client 
    * @brief Clients removal function.
    * 
    * @param epollfd
    * @param client_socket 
    * @param clients_list 
    * @return void 
    */
    void disconnect_client(int epollfd, int client_socket, client_list_t *clients_list);

    /*! @fn admin_cmd_mngt 
    * @brief Gets and execute admin command.
    * 
    * @param void
    * @return void 
    */
    void admin_cmd_mngt(void);

    /*! @fn receive_msg 
    * @brief Receive a message from a fd.
    * 
    * @param fd
    * @return void 
    */
    char *receive_msg(int fd);

    /*! @fn buffer_msg 
    * @brief Receive messages and buffer them.
    * 
    * @param client 
    * @return void 
    */
   void buffer_msg(client_t *client);

    /*! @fn send_msg 
    * @brief send a message to a socket.
    * 
    * @param socket 
    * @param msg 
    * @return void 
    */
    void send_msg(int socket, char *msg);

    /*! @fn unbuffer_msg 
    * @brief Send all waiting messages to a client.
    * 
    * @param client
    * @return void 
    */
    void unbuffer_msg(client_t *client);

#endif