/**
 * @file subscriber_utils.c
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

#include "./include/subscriber_utils.h"

/**
 * @brief Inits the TCP connection to the server and send the client ID.
 * 
 * @param client client structure.
 * @param ip server ip number in dotted standart.
 * @param hport server port number.
 * @return int 0 if tcp init went successfully or -1 otherwise.
 */
static int init_client_tcp_socket(client_t *client, const char *ip, const uint16_t hport) {
    if ((client->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return client->tcp_socket;
    }

    memset(&client->tcp_addr, 0, sizeof client->tcp_addr);

    client->tcp_addr.sin_family = AF_INET;
    client->tcp_addr.sin_addr.s_addr = inet_addr(ip);
    client->tcp_addr.sin_port = htons(hport);

    /* Connect to the server side */
    if (connect(client->tcp_socket, (const struct sockaddr *) &client->tcp_addr, sizeof client->tcp_addr) < 0) {
        return -1;
    }

    client->send_msg->len = strlen(client->id) + 1;
    strcpy(client->send_msg->data, client->id);
    
    /* Server accepted the connection send the ID */
    if (send_tcp_msg(client->tcp_socket, (void *)client->send_msg, sizeof *client->send_msg) != OK) {
        return -1;
    }

    return 0;
}

/**
 * @brief Inits the poll vector for the client and adds the stdin and the TCP server socket.
 * 
 * @param client client structure.
 * @return int 0 if poll vector was initialized successfully or -1 otherwise.
 */
static int init_client_poll_vec(client_t *client) {
    if (create_poll_vec(&client->poll_vec, 2) != OK) {
        return -1;
    }

    /* Add stdin file descriptor */
    if (poll_vec_add_fd(client->poll_vec, STDIN_FILENO, POLLIN) != OK) {
        free_poll_vec(&client->poll_vec);
        return -1;
    }

    /* Add the TCP server socket file descriptor */
    if (poll_vec_add_fd(client->poll_vec, client->tcp_socket, POLLIN | POLLOUT) != OK) {
        free_poll_vec(&client->poll_vec);
        return -1;
    }

    return 0;
}

/**
 * @brief Inits the required buffers, structures in order to maintain the
 * connection with the server as protocol packages structures to send and
 * receive messages over the TCP connection.
 * 
 * @param client client structure.
 * @return int 0 if the allocation went successfully or -1 otherwise.
 */
static int init_client_buffers(client_t *client) {
    client->cmd = malloc(sizeof *client->cmd * MAX_CLIENT_CMD_LEN);
    if (client->cmd == NULL) {
        return -1;
    }

    client->send_msg = malloc(sizeof *client->send_msg);
    if (client->send_msg == NULL) {
        free(client->cmd);

        return -1;
    }

    client->recv_msg = malloc(sizeof *client->recv_msg);
    if (client->recv_msg == NULL) {
        free(client->cmd);
        free(client->send_msg);

        return -1;
    }
    
    /* Clear junk bytes from the structures */
    memset(client->cmd, 0, MAX_CLIENT_CMD_LEN);
    memset(client->send_msg, 0, sizeof *client->send_msg);
    memset(client->recv_msg, 0, sizeof *client->recv_msg);

    return 0;
}

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
err_t init_client(client_t **client, const char *id, const char *ip, const uint16_t hport) {
    if (*client != NULL) {
        return CLIENT_INPUT_IS_NOT_NULL;
    }

    if ((ip == NULL) || (id == NULL)) {
        return CLIENT_INPUT_CONNECT_IS_NULL;
    }

    if (hport == 0) {
        return INVALID_PORT_NUMBER;
    }

    *client = malloc(sizeof **client);
    if (*client == NULL) {
        return CLIENT_FAILED_ALLOCATION;
    }

    (*client)->id = malloc(sizeof *(*client)->id * MAX_ID_CLIENT_LEN);
    if ((*client)->id == NULL) {
        free(*client);
        *client = NULL;

        return CLIENT_FAILED_ALLOCATION;
    }

    strcpy((*client)->id, id);

    if (init_client_buffers(*client) < 0) {
        free(*client);
        *client = NULL;

        return CLIENT_FAILED_ALLOCATION;
    }

    if (init_client_tcp_socket(*client, ip, hport) < 0) {
        free((*client)->id);
        free((*client)->cmd);
        free((*client)->send_msg);
        free((*client)->recv_msg);
        free(*client);
        *client = NULL;

        return CLIENT_FAILED_TCP;
    }

    (*client)->poll_vec = NULL;
    if (init_client_poll_vec(*client) < 0) {
        close((*client)->tcp_socket);
        free((*client)->id);
        free((*client)->cmd);
        free((*client)->send_msg);
        free((*client)->recv_msg);
        free(*client);
        *client = NULL;

        return CLIENT_FAILED_POLL_VEC;
    }
    
    return OK;
}

/**
 * @brief Frees the memory allocated for a client structure and closes the
 * connection with the server.
 * 
 * @param client pointer to client structure MUST not be NULL.
 * @return err_t OK if client was freed successfully and connection was closed
 * or error otherwise.
 */
err_t free_client(client_t **client) {
    if ((client == NULL) || (*client == NULL)) {
        return CLIENT_INPUT_IS_NULL;
    }

    if ((*client)->poll_vec != NULL) {
        free_poll_vec(&(*client)->poll_vec);
    }

    if ((*client)->id != NULL) {
        free((*client)->id);
    }

    if ((*client)->cmd != NULL) {
        free((*client)->cmd);
    }

    if ((*client)->send_msg != NULL) {
        free((*client)->send_msg);
    }

    if ((*client)->recv_msg != NULL) {
        free((*client)->recv_msg);
    }

    free(*client);
    *client = NULL;

    return OK;
}

/**
 * @brief Poll the available fds, the poll timeout is set to -1.
 * If the function returns with POLL_FAILED_TIMED_OUT the connection
 * is wrong or the fds is unavilable.
 * 
 * @param this client structure.
 * @return err_t OK if atleast one fd is available for specified events.
 */
err_t wait_for_ready_fds(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    /* Should not timeout */
    if (poll(this->poll_vec->pfds, this->poll_vec->nfds, -1) <= 0) {
        return POLL_FAILED_TIMED_OUT; 
    }

    return OK;
}

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
err_t process_ready_fds(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    err_t err = OK;

    if ((this->poll_vec->pfds[1].revents & POLLIN) != 0) {

        /* Receive an encapsulated TCP protocol message from the server side */
        err = recv_tcp_msg(this->tcp_socket, (void *)this->recv_msg, sizeof *this->recv_msg);

        if (err == TCP_FAILED_SEND_RECV) {
            /* Server closed the connection so exit the main thread */

            return OK_WITH_EXIT;
        } else if (err != OK) {
            return err;
        } else {
            /* Print the data received from the server which is a topic data */

            printf("%s\n", this->recv_msg->data);
        }
    }

    return OK;
}

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
err_t process_subscribe_cmd(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    char *save_ptr = NULL;

    /* Cannot be NULL checked on get_cmd_line function */
    char *cmd = __strtok_r(this->cmd, WORD_SEPARATOR, &save_ptr);

    /* If NULL exit the subscribe cmd */
    char *topic = __strtok_r(NULL, WORD_SEPARATOR, &save_ptr);

    if (topic == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    /* Match the invalid input with a valid sf number */
    char *sf_str = __strtok_r(NULL, WORD_SEPARATOR, &save_ptr);
    uint8_t sf = sf_str == NULL ? 0 : (atoi(sf_str) <= 0 ? 0 : 1);

    /* Copy the input data into the send_msg package TCP protocol */

    size_t send_offset = 0;

    this->send_msg->len = strlen(cmd) + 1;
    strcpy(this->send_msg->data, cmd);

    send_offset += this->send_msg->len;
    this->send_msg->len = strlen(topic) + 1;
    strcpy(this->send_msg->data + send_offset, topic);

    send_offset += this->send_msg->len;
    this->send_msg->len = sizeof sf;
    memcpy(this->send_msg->data + send_offset, &sf, this->send_msg->len);

    this->send_msg->len += send_offset;

    /* Send the request to the server side */
    return send_tcp_msg(this->tcp_socket, (void *)this->send_msg, sizeof *this->send_msg);
}

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
err_t process_unsubscribe_cmd(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    char *save_ptr = NULL;

    /* Cannot be NULL checked on get_cmd_line function */
    char *cmd = __strtok_r(this->cmd, WORD_SEPARATOR, &save_ptr);

    /* If NULL exit the unsubscribe cmd */
    char *topic = __strtok_r(NULL, WORD_SEPARATOR, &save_ptr);

    if (topic == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    /* Copy the input data into the send_msg package TCP protocol */

    size_t send_offset = 0;

    this->send_msg->len = strlen(cmd) + 1;
    strcpy(this->send_msg->data, cmd);

    send_offset += this->send_msg->len;
    this->send_msg->len = strlen(topic) + 1;
    strcpy(this->send_msg->data + send_offset, topic);

    this->send_msg->len += send_offset;

    /* Send the request to the server side */
    return send_tcp_msg(this->tcp_socket, (void *)this->send_msg, sizeof *this->send_msg);
}

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
cmd_line_t get_cmd_line(client_t *this) {
    if (this == NULL) {
        return NONE;
    }

    if (this->cmd == NULL) {
        return NONE;
    }

    if (this->poll_vec == NULL) {
        return NONE;
    }

    if (this->poll_vec->pfds[0].fd == STDIN_FILENO) {
        if ((this->poll_vec->pfds[0].revents & POLLIN) != 0) {
            if (fgets(this->cmd, MAX_CLIENT_CMD_LEN, stdin) != NULL) {
                if (strncmp(this->cmd, SUBSCRIBE_CMD, SUBSCRIBE_CMD_LEN) == 0) {
                    return SUBSCRIBE;
                } else if (strncmp(this->cmd, UNSUBSCRIBE_CMD, UNSUBSCRIBE_CMD_LEN) == 0) {
                    return UNSUBSCRIBE;
                } else if (strncmp(this->cmd, EXIT_CMD, EXIT_CMD_LEN) == 0) {
                    return EXIT;
                } else {
                    return NONE;
                }
            }
        }
    }

    return NONE;
}
