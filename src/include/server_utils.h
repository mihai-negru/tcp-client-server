/**
 * @file server_utils.h
 * @author Mihai Negru (determinant289@gmail.com)
 * @version 1.0.0
 * @date 2023-05-02
 *
 * @copyright Copyright (C) 2023-2024 Mihai Negru <determinant289@gmail.com>
 * This file is part of tcp-client-server.
 *
 * tcp-client-server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tcp-client-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tcp-client-server.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include "./utils.h"
#include "./poll_vec.h"
#include "./udp_type.h"
#include "./tcp_type.h"
#include "./client_vec.h"

#define MAX_SERVER_BUFLEN       8096
#define MAX_LISTEN_SOCKET       10

#define INIT_NFDS               (nfds_t)10
#define INIT_CLIENTS            10
#define INIT_UDP_MSGS_LEN       20

#define EXIT_CMD                "exit\0"
#define EXIT_CMD_LEN            strlen(EXIT_CMD)

typedef struct server_s {
    int                     udp_socket;         /* UDP socket to get udp messages */
    int                     tcp_socket;         /* Listener tcp socket for subscribers */
    struct sockaddr_in      udp_addr;
    struct sockaddr_in      tcp_addr;
    struct sockaddr_in      udp_addr_client;
    poll_vec_t              *poll_vec;          /* Poll vector with all active fds */
    char                    *buf;               /* Buffer to process udp messages */
    char                    *cmd;               /* Buffer to process user input commands */
    tcp_msg_t               *send_msg;          /* Encapsulated TCP msg protocol for sending */
    tcp_msg_t               *recv_msg;          /* Encapsulated TCP msg protocol for receiving */
    client_vec_t            *clients;           /* Clients vector containg all clients metadata */
    udp_type_t              *udp_msgs;          /* Available udp type class containg topics */
    size_t                  udp_msgs_len;
    size_t                  udp_msgs_capacity;
} server_t;

/**
 * @brief Inits the server by binding two sockets one for UDP and one for TCP connection.
 *
 *
 * @param server pointer to server structure, MUST be NULL.
 * @param hport a valid port number to open the connection.
 * @return err_t OK if the server was allocated and initialized successfully or
 * error otherwise.
 */
err_t init_server(server_t **server, const uint16_t hport);

/**
 * @brief Frees the resources allocated by the server and closes all the connections
 * which generates closing actions for every active client.
 *
 * @param server pointer to server structure, MUST not be NULL.
 * @return err_t OK if all the resources were freed and all connections
 * were closed or error otherwise.
 */
err_t free_server(server_t **server);

/**
 * @brief Poll the available fds, the poll timeout is set to -1.
 * If the function returns with POLL_FAILED_TIMED_OUT the connection
 * is wrong or the fds is unavilable.
 *
 * @param this server structure.
 * @return err_t OK if atleast one fd is available for specified events.
 */
err_t wait_for_ready_fds(server_t *this);

/**
 * @brief Receives a message from the client and processes it.
 * The stdin fd is NOT processed here.
 * Message can be processed both from udp and tcp sockets, if
 * the ready socket is udp then the message will be processed and a POLLOUT
 * will be generated for the clients subscribed at the desired topic.
 * The udp message is saved internally in case a new client subscribes to the topic,
 * ot the udp message was not send due to closed connection (SF functionality).
 *
 * In case of a TCP POLLIN the server will process the message without sending
 * any messages back.
 *
 * @param this server structure.
 * @return err_t OK if the messages were processed successfully or error otherwise.
 */
err_t process_ready_fds(server_t *this);

/**
 * @brief Receives an input command from the stdin and processes it
 * by checking if the command is exit.
 *
 * @param this server structure.
 * @return uint8_t 1 if command is exit or 0 otherwise.
 */
uint8_t check_if_exit(server_t *this);

#endif /* SERVER_UTILS_H_ */
