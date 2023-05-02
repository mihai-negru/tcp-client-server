/**
 * @file subscriber_utils.h
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

#ifndef SUBSCRIBER_UTILS_H_
#define SUBSCRIBER_UTILS_H_

#include "./utils.h"
#include "./poll_vec.h"
#include "./tcp_type.h"

#define MAX_CLIENT_CMD_LEN      128
#define WORD_SEPARATOR          " \n"

#define SUBSCRIBE_CMD           "subscribe\0"
#define SUBSCRIBE_CMD_LEN       strlen(SUBSCRIBE_CMD)

#define UNSUBSCRIBE_CMD         "unsubscribe\0"
#define UNSUBSCRIBE_CMD_LEN     strlen(UNSUBSCRIBE_CMD)

#define EXIT_CMD                "exit\0"
#define EXIT_CMD_LEN            strlen(EXIT_CMD)

/**
 * @brief Enum class type to maintain valid
 * command definitions.
 * 
 */
typedef enum cmd_line_s {
    SUBSCRIBE       = 0,
    UNSUBSCRIBE     = 1,
    EXIT            = 2,
    NONE            = 3
} cmd_line_t;

/**
 * @brief Structure type class defining
 * a clients (subscriber) type, containg
 * data and metadata for processing responses
 * and requests.
 * 
 */
typedef struct client_s {
    char                    *id;            /* Unique client ID */
    int                     tcp_socket;     /* Server TCP socket */
    struct sockaddr_in      tcp_addr;       /* Server informations */
    poll_vec_t              *poll_vec;      /* Poll vector structure */
    char                    *cmd;           /* Buffer for reading stdin commands */
    tcp_msg_t               *send_msg;      /* Encapsulated TCP msg protocol for sending */
    tcp_msg_t               *recv_msg;      /* Encapsulated TCP msg protocol for receiving */
} client_t;

/**
 * @brief Allocates a client type structure and connects to the server.
 * Upon accepting the connection the client will send a TCP message with
 * its ID, after getting the ID the server will decide if the client has
 * the permission to send requests, if nothing is sent back from the server
 * the client is available to send request, otherwise the client will shut
 * itself. 
 * 
 * @param client pointer to client structure in otder to allocate, MUST be NULL. 
 * @param id string ID name assigned to a client.
 * @param ip server ip representation with dotted standart.
 * @param hport server port number must be a valid port.
 * @return err_t OK if the client was allocated and has permissions to send requests
 * to the server or error otherwise.
 */
err_t init_client(client_t **client, const char *id, const char *ip, const uint16_t hport);

/**
 * @brief Frees the memory allocated for a client structure and closes the
 * connection with the server.
 * 
 * @param client pointer to client structure MUST not be NULL.
 * @return err_t OK if client was freed successfully and connection was closed
 * or error otherwise.
 */
err_t free_client(client_t **client);

/**
 * @brief Poll the available fds, the poll timeout is set to -1.
 * If the function returns with POLL_FAILED_TIMED_OUT the connection
 * is wrong or the fds is unavilable.
 * 
 * @param this client structure.
 * @return err_t OK if atleast one fd is available for specified events.
 */
err_t wait_for_ready_fds(client_t *this);

/**
 * @brief Receives a message from the server and processes it.
 * The stdin fd is NOT processed here, just the server socket.
 * The client prints the server message without any additional checks,
 * because all the checks are made on the server side (supposing we have
 * a sequare and error prone connection).
 * 
 * @param this client structure.
 * @return err_t OK if message was received successfully,
 * OK_WITH_EXIT if server closed the connection with the client or
 * error otherwise.
 */
err_t process_ready_fds(client_t *this);

/**
 * @brief Processes a subscribe message from the stdin.
 * The input must be a valid one, however client runs a series
 * of checks in order to send a valid request to the server, so
 * mismatches from the input are converted to the most significat
 * valid input.
 * 
 * @param this client structure.
 * @return err_t OK if the message was processed and sent successfully to the server
 * or error otherwise.
 */
err_t process_subscribe_cmd(client_t *this);

/**
 * @brief Processes an unsubscribe message from the stdin.
 * The input must be a valid one, however client runs a series
 * of checks in order to send a valid request to the server, so
 * mismatches from the input are converted to the most significat
 * valid input.
 * 
 * @param this client structure.
 * @return err_t OK if the message was processed and sent successfully to the server
 * or error otherwise.
 */
err_t process_unsubscribe_cmd(client_t *this);

/**
 * @brief A POLLIN for the stdin was send out so process the command.
 * The command should contain a valid command descriptor, if the command
 * name is not valid a `NONE` command will be sent to process, in other words
 * the command will be NOT processed by the subscriber.
 *
 * This function checks just for command descriptor not for valid input,
 * the input checks are made in prcoess_* functions.s
 * 
 * @param this client structure.
 * @return cmd_line_t valid command descriptor.
 */
cmd_line_t get_cmd_line(client_t *this);

#endif /* SUBSCRIBER_UTILS_H_ */
