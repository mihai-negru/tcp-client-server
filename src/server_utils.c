/**
 * @file server_utils.c
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

#include "./include/server_utils.h"

/**
 * @brief Inits a UDP socket for the server.
 *
 * @param server server structure.
 * @param hport server port number.
 * @return int 0 if init went successfully or -1 otherwise.
 */
static int init_server_udp_socket(server_t *server, const uint16_t hport) {
    if ((server->udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        return server->udp_socket;
    }

    memset(&server->udp_addr, 0, sizeof server->udp_addr);

    server->udp_addr.sin_family = AF_INET;
    server->udp_addr.sin_addr.s_addr = INADDR_ANY;
    server->udp_addr.sin_port = htons(hport);

    /* Bind the UDP socket */
    if (bind(
        server->udp_socket,
        (const struct sockaddr *) &server->udp_addr,
        sizeof server->udp_addr) < 0
    ) {
        return -1;
    }

    return 0;
}

/**
 * @brief Inits a TCP socket for the server.
 *
 * @param server server structure.
 * @param hport server port number.
 * @return int 0 if init went successfully or -1 otherwise.
 */
static int init_server_tcp_socket(server_t *server, const uint16_t hport) {
    if ((server->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return server->tcp_socket;
    }

    memset(&server->tcp_addr, 0, sizeof server->tcp_addr);

    server->tcp_addr.sin_family = AF_INET;
    server->tcp_addr.sin_addr.s_addr = INADDR_ANY;
    server->tcp_addr.sin_port = htons(hport);

    if (bind(
        server->tcp_socket,
        (const struct sockaddr *) &server->tcp_addr,
        sizeof server->tcp_addr) < 0
    ) {
        return -1;
    }

    if (listen(server->tcp_socket, MAX_LISTEN_SOCKET) < 0) {
        return -1;
    }

    /* Disable Nagle algorithm for the TCP socket */
    if (setsockopt(server->tcp_socket, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof (int)) < 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Inits the poll vector for the server and adds the stdin and UDP, TCP server socket.
 *
 * @param server server structure.
 * @return int 0 if poll vector was initialized successfully or -1 otherwise.
 */
static int init_server_poll_vec(server_t *server) {
    if (create_poll_vec(&server->poll_vec, INIT_NFDS) != OK) {
        return -1;
    }

    /* Add stdin file descriptor */
    if (poll_vec_add_fd(server->poll_vec, STDIN_FILENO, POLLIN) != OK) {
        free_poll_vec(&server->poll_vec);
        return -1;
    }

    /* Add the UDP server socket file descriptor */
    if (poll_vec_add_fd(server->poll_vec, server->udp_socket, POLLIN) != OK) {
        free_poll_vec(&server->poll_vec);
        return -1;
    }

    /* Add the TCP server socket file descriptor */
    if (poll_vec_add_fd(server->poll_vec, server->tcp_socket, POLLIN) != OK) {
        free_poll_vec(&server->poll_vec);
        return -1;
    }

    return 0;
}

/**
 * @brief Inits the required buffers, structures in order to maintain the
 * connection with the clients as protocol packages structures to send and
 * receive messages over the TCP connection.
 *
 * @param server server structure.
 * @return int 0 if the allocation went successfully or -1 otherwise.
 */
static int init_server_buffers(server_t *server) {
    server->buf = malloc(sizeof *server->buf * MAX_SERVER_BUFLEN);
    if (server->buf == NULL) {
        return -1;
    }

    server->cmd = malloc(sizeof *server->cmd * MAX_CMD_LEN);
    if (server->cmd == NULL) {
        free(server->buf);

        return -1;
    }

    server->send_msg = malloc(sizeof *server->send_msg);
    if (server->send_msg == NULL) {
        free(server->buf);
        free(server->cmd);

        return -1;
    }

    server->recv_msg = malloc(sizeof *server->recv_msg);
    if (server->recv_msg == NULL) {
        free(server->buf);
        free(server->cmd);
        free(server->send_msg);

        return -1;
    }

    server->udp_msgs_len = 0;
    server->udp_msgs_capacity = INIT_UDP_MSGS_LEN;
    server->udp_msgs = malloc(sizeof *server->udp_msgs * INIT_UDP_MSGS_LEN);
    if (server->udp_msgs == NULL) {
        free(server->buf);
        free(server->cmd);
        free(server->send_msg);
        free(server->recv_msg);

        return -1;
    }

    /* Clear junk bytes from the structures */
    memset(server->buf, 0, MAX_SERVER_BUFLEN);
    memset(server->cmd, 0, MAX_CMD_LEN);
    memset(server->send_msg, 0, sizeof *server->send_msg);
    memset(server->recv_msg, 0, sizeof *server->recv_msg);

    return 0;
}

/**
 * @brief Inits the server by binding two sockets one for UDP and one for TCP connection.
 *
 * @param server pointer to server structure, MUST be NULL.
 * @param hport a valid port number to open the connection.
 * @return err_t OK if the server was allocated and initialized successfully or
 * error otherwise.
 */
err_t init_server(server_t **server, const uint16_t hport) {
    if (*server != NULL) {
        return SERVER_INPUT_IS_NOT_NULL;
    }

    if (hport == 0) {
        return INVALID_PORT_NUMBER;
    }

    *server = malloc(sizeof **server);
    if (*server == NULL) {
        return SERVER_FAILED_ALLOCATION;
    }

    if (init_server_buffers(*server) < 0) {
        free(*server);
        *server = NULL;

        return SERVER_FAILED_ALLOCATION;
    }

    if (init_server_udp_socket(*server, hport) < 0) {
        free((*server)->buf);
        free((*server)->cmd);
        free((*server)->send_msg);
        free((*server)->recv_msg);
        free((*server)->udp_msgs);
        free(*server);
        *server = NULL;

        return SERVER_FAILED_UDP;
    }

    if (init_server_tcp_socket(*server, hport) < 0) {
        close((*server)->udp_socket);
        free((*server)->buf);
        free((*server)->cmd);
        free((*server)->send_msg);
        free((*server)->recv_msg);
        free((*server)->udp_msgs);
        free(*server);
        *server = NULL;

        return SERVER_FAILED_TCP;
    }

    (*server)->poll_vec = NULL;
    if (init_server_poll_vec(*server) < 0) {
        close((*server)->udp_socket);
        close((*server)->tcp_socket);
        free((*server)->buf);
        free((*server)->cmd);
        free((*server)->send_msg);
        free((*server)->recv_msg);
        free((*server)->udp_msgs);
        free(*server);
        *server = NULL;

        return SERVER_FAILED_POLL_VEC;
    }

    (*server)->clients = NULL;
    if (create_clients_vec(&(*server)->clients, INIT_CLIENTS) != OK) {
        close((*server)->udp_socket);
        close((*server)->tcp_socket);
        free((*server)->buf);
        free((*server)->cmd);
        free((*server)->send_msg);
        free((*server)->recv_msg);
        free((*server)->udp_msgs);
        free_poll_vec(&(*server)->poll_vec);
        free(*server);
        *server = NULL;

        return SERVER_FAILED_CLIENTS_VEC;
    }

    return OK;
}

/**
 * @brief Frees the resources allocated by the server and closes all the connections
 * which generates closing actions for every active client.
 *
 * @param server pointer to server structure, MUST not be NULL.
 * @return err_t OK if all the resources were freed and all connections
 * were closed or error otherwise.
 */
err_t free_server(server_t **server) {
    if ((server == NULL) || (*server == NULL)) {
        return SERVER_INPUT_IS_NULL;
    }

    if ((*server)->poll_vec != NULL) {
        free_poll_vec(&(*server)->poll_vec);
    }

    if ((*server)->buf != NULL) {
        free((*server)->buf);
    }

    if ((*server)->cmd != NULL) {
        free((*server)->cmd);
    }

    if ((*server)->send_msg != NULL) {
        free((*server)->send_msg);
    }

    if ((*server)->recv_msg != NULL) {
        free((*server)->recv_msg);
    }

    if ((*server)->udp_msgs != NULL) {
        free((*server)->udp_msgs);
    }

    if ((*server)->clients != NULL) {
        free_clients_vec(&(*server)->clients);
    }

    free(*server);
    *server = NULL;

    return OK;
}

/**
 * @brief Poll the available fds, the poll timeout is set to -1.
 * If the function returns with POLL_FAILED_TIMED_OUT the connection
 * is wrong or the fds is unavilable.
 *
 * @param this server structure.
 * @return err_t OK if atleast one fd is available for specified events.
 */
err_t wait_for_ready_fds(server_t *this) {
    if (this == NULL) {
        return SERVER_INPUT_IS_NULL;
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
 * @brief Parses a udp_type_t class into a continuous buffer to send over
 * a TCP connection.
 *
 * @param this server structure.
 * @param msg pointer to udp_type_t class in order to parse.
 * @return err_t OK if message was parsed correctly or
 * error otherwise.
 */
static err_t pack_topic_to_tcp_msg(server_t *this, udp_type_t *msg) {
    this->send_msg->len = snprintf(
        this->send_msg->data,
        MAX_TCP_MSG_BUF_LEN,
        "%s:%hu - %s -",
        inet_ntoa(msg->addr.sin_addr),
        ntohs(msg->addr.sin_port),
        msg->topic
    );

    switch (msg->type) {
        case INT:
            this->send_msg->len += snprintf(
                this->send_msg->data + this->send_msg->len,
                MAX_TCP_MSG_BUF_LEN - this->send_msg->len,
                " INT - %d", msg->data.INT
            );
            break;
        case SHORT_REAL:
            this->send_msg->len += snprintf(
                this->send_msg->data + this->send_msg->len,
                MAX_TCP_MSG_BUF_LEN - this->send_msg->len,
                " SHORT_REAL - %.2f", msg->data.SHORT_REAL
            );
            break;
        case FLOAT:
            this->send_msg->len += snprintf(
                this->send_msg->data + this->send_msg->len,
                MAX_TCP_MSG_BUF_LEN - this->send_msg->len,
                " FLOAT - %f", msg->data.FLOAT
            );
            break;
        case STRING:
            this->send_msg->len += snprintf(
                this->send_msg->data + this->send_msg->len,
                MAX_TCP_MSG_BUF_LEN - this->send_msg->len,
                " STRING - %s", msg->data.STRING
            );
            break;
        default:
            return UDP_UNKNOWN_DATA_TYPE;
    }

    return OK;
}

/**
 * @brief Processes a TCP message from the client side, the message
 * is parsed into internal structures and additional function are called
 * to process the request (subscribe/unsubscribe).
 *
 * @param this server structure.
 * @param client_fd valid tcp socket file descriptor assigned for a client.
 * @return err_t OK if the message was processed successfully or error otherwise.
 */
static err_t process_server_tcp_msg(server_t *this, int client_fd) {
    char *cmd = this->recv_msg->data;
    char *topic = this->recv_msg->data + strlen(cmd) + 1;

    if (strcmp(cmd, "subscribe") == 0) {
        /* Process a subscribe action */

        return subscribe_client_to_topic(
            this->clients,
            client_fd,
            topic,
            *(uint8_t *)(this->recv_msg->data + this->recv_msg->len - 1) == 0 ? NO_SF : SF
        );
    } else if (strcmp(cmd, "unsubscribe") == 0) {
        /* Process an unsubscribe action */

        return unsubscribe_client_from_topic(
            this->clients,
            client_fd,
            topic
        );
    } else {
        return SERVER_UNKNOWN_COMMAND;
    }

    return OK;
}

/**
 * @brief Adds a UDP message into the udp messages queue.
 * Internal local storage to store udp messages for futher
 * processing or for Store and Forward functionality.
 *
 * @param this server structure.
 * @return err_t OK if the udp message was added successfully or
 * error otherwise.
 */
static err_t add_server_udp_msg(server_t *this) {
    err_t err = OK;

    /* Adds more memory for new udp messages */
    if (this->udp_msgs_len == this->udp_msgs_capacity) {
        udp_type_t *udp_msgs_real = realloc(
            this->udp_msgs,
            sizeof *this->udp_msgs * this->udp_msgs_capacity * REALLOC_FACTOR
        );

        if (udp_msgs_real == NULL) {
            return SERVER_COULD_NOT_ADD_NEW_UDP;
        }

        this->udp_msgs = udp_msgs_real;
        this->udp_msgs_capacity *= REALLOC_FACTOR;
    }

    /*
     * Clear the current space for the udp message
     * and parse the informations into it
     */
    memset(&this->udp_msgs[this->udp_msgs_len], 0, sizeof *this->udp_msgs);
    if ((err = parse_udp_type_from(
        &this->udp_msgs[this->udp_msgs_len],
        &this->udp_addr_client,
        this->buf)) != OK
    ) {
        return err;
    }

    (this->udp_msgs_len)++;

    return OK;
}

/**
 * @brief Sends a new topic message to all subscribed clients.
 * If one client is disconnected, but has the store-and-forward
 * functionality the a pointer to the message will be stacked
 * in a local client queue and upon reconnection the message will be sent.
 *
 * @param this server structure.
 * @return err_t OK if the udp message was sent to all active clients or
 * error otherwise.
 */
static err_t transmit_topic_to_clients(server_t *this) {
    err_t err = OK;

    /* Get the message ready for shipping */
    if ((err = pack_topic_to_tcp_msg(this, &this->udp_msgs[this->udp_msgs_len - 1]))) {
        return err;
    }

    /* Iterate over all active/dead clients */
    for (size_t iter = 0; iter < this->clients->len; ++iter) {

        /* Iterate over client's topics */
        for (size_t iter_j = 0; iter_j < this->clients->entities[iter].topics_len; ++iter_j) {
            if (strcmp(
                this->udp_msgs[this->udp_msgs_len - 1].topic,
                this->clients->entities[iter].topics[iter_j]) == 0
            ) {
                /* Client is subscribed to the received topic */

                if (this->clients->entities[iter].status == ACTIVE) {
                    /* Client is active, send the package */

                    if ((err = send_tcp_msg(
                        this->clients->entities[iter].fd,
                        (void *)this->send_msg,
                        sizeof *this->send_msg)) != OK
                    ) {
                        debug_msg(err);
                    }
                } else if (this->clients->entities[iter].options[iter_j] == SF) {
                    /* Client is dead, however has the store-and-forward option */

                    if ((err = add_topic_msg_for_client(
                        this->clients,
                        &this->udp_msgs[this->udp_msgs_len - 1],
                        iter)) != OK
                    ) {
                        return err;
                    }
                }

                /*
                 * If the topic was found do not
                 * iterate over the rest of topics
                 */
                break;
            }
        }
    }

    return OK;
}

/**
 * @brief Upon reconnecting with a client the stacked messages will
 * will be sent to the client. If the client closes its connection the
 * rest of the messages from the queue will wait for another reconnection.
 *
 * The process repeats every time there is a reconnection and any messages
 * are ready for sending.
 *
 * @param this server structure.
 * @param client_fd new valid tcp socket file descriptor for the client.
 * @return err_t OK if any of the udp messages were send to the client
 * or error otherwise.
 */
static err_t retransmit_topics_to_client(server_t *this, int client_fd) {
    err_t err = OK;

    /* Check if the client was ever connected to the server */
    for (size_t iter = 0; iter < this->clients->len; ++iter) {
        if (this->clients->entities[iter].fd == client_fd) {
            size_t remaining_msgs = this->clients->entities[iter].ready_msgs_len;

            /*
             * Start sending topics messages until sent all
             * or interrupted by closing connection
             */
            for (; remaining_msgs > 0; remaining_msgs--) {

                /* Pack the message */
                if ((err = pack_topic_to_tcp_msg(this, this->clients->entities[iter].ready_msgs[remaining_msgs - 1]))) {
                    this->clients->entities[iter].ready_msgs_len = remaining_msgs;
                    return err;
                }

                /* Send the message over the client file descriptor */
                if ((err = send_tcp_msg(client_fd, (void *)this->send_msg, sizeof *this->send_msg)) != OK) {
                    this->clients->entities[iter].ready_msgs_len = remaining_msgs;
                    debug_msg(err);

                    break;
                }
            }

            /* No udp messages left */
            this->clients->entities[iter].ready_msgs_len = 0;

            return OK;
        }
    }

    /* Client could not be found in the server */
    return CLIENTS_VEC_CLIENT_NOT_FOUND;
}

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
err_t process_ready_fds(server_t *this) {
    if (this == NULL) {
        return SERVER_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    err_t err = OK;
    for (nfds_t iter = 1; iter < this->poll_vec->nfds; ++iter) {
        if ((this->poll_vec->pfds[iter].revents & POLLIN) != 0) {
            if (this->poll_vec->pfds[iter].fd == this->udp_socket) {
                /* Process an UDP message */

                ssize_t udp_bytes = recvfrom(
                    this->udp_socket,
                    this->buf,
                    MAX_SERVER_BUFLEN,
                    0,
                    (struct sockaddr *) &this->udp_addr_client,
                    &(socklen_t){sizeof this->udp_addr_client}
                );

                if (udp_bytes == 0) {
                    if ((err = poll_vec_remove_fd(this->poll_vec, iter)) != OK) {
                        return err;
                    }

                    iter--;
                } else {

                    /* Add udp message to the server local storage */
                    if ((err = add_server_udp_msg(this)) != OK) {
                        return err;
                    }

                    /* Transmit the udp message according to above protocol */
                    if ((err = transmit_topic_to_clients(this)) != OK) {
                        return err;
                    }
                }
            } else if (this->poll_vec->pfds[iter].fd == this->tcp_socket) {
                /* Connect a new client to the server */

                struct sockaddr_in new_client;
                memset(&new_client, 0, sizeof new_client);

                int new_client_fd = accept(
                    this->tcp_socket,
                    (struct sockaddr *) &new_client,
                    &(socklen_t){sizeof new_client}
                );

                if (new_client_fd <= 0) {
                    return SERVER_FAILED_ACCEPT_TCP;
                } else {
                    if ((err = poll_vec_add_fd(
                        this->poll_vec,
                        new_client_fd,
                        POLLIN | POLLOUT)) != OK
                    ) {
                        return err;
                    }

                    /* Receive the client's ID */
                    if ((err = recv_tcp_msg(
                        new_client_fd,
                        (void *)this->recv_msg,
                        sizeof *this->recv_msg)) != OK
                    ) {
                        return SERVER_COULD_NOT_CONNECT_NEW_CLIENT;
                    } else {
                        if ((err = register_new_client(
                            this->clients,
                            this->recv_msg->data,
                            new_client_fd)) != OK
                        ) {
                            /* Client is already connected with the specified ID */

                            poll_vec_remove_fd(this->poll_vec, this->poll_vec->nfds - 1);
                            printf("Client %s already connected.\n", this->recv_msg->data);
                        } else {
                            /* New client arrived or a dead client is reconnected */

                            printf(
                                "New client %s connected from %s:%hu.\n",
                                this->recv_msg->data,
                                inet_ntoa(new_client.sin_addr),
                                ntohs(new_client.sin_port)
                            );

                            /* If the client is reconnecting retransmit the topic messages */
                            if ((err = retransmit_topics_to_client(this, new_client_fd)) != OK) {
                                debug_msg(err);
                            }
                        }
                    }
                }
            } else {
                /* Process a client TCP message */

                if ((err = recv_tcp_msg(
                    this->poll_vec->pfds[iter].fd,
                    (void *)this->recv_msg,
                    sizeof *this->recv_msg)) != OK
                ) {
                    /*
                     * Could not receive the message,
                     * it means the client has closed the connection
                     * so change the status of the client
                     */

                    size_t close_client_idx = 0;
                    if ((err = close_active_client(
                        this->clients,
                        this->poll_vec->pfds[iter].fd,
                        &close_client_idx)) != OK
                    ) {
                        debug_msg(err);
                    } else {
                        poll_vec_remove_fd(this->poll_vec, iter);
                        iter--;

                        printf(
                            "Client %s disconnected.\n",
                            get_client_id(this->clients,
                            close_client_idx)
                        );
                    }
                } else {
                    /* Message received successfully, process it */

                    if ((err = process_server_tcp_msg(
                        this,
                        this->poll_vec->pfds[iter].fd)) != OK
                    ) {
                        debug_msg(err);
                    }
                }
            }
        }
    }

    return OK;
}

/**
 * @brief Receives an input command from the stdin and processes it
 * by checking if the command is exit.
 *
 * @param this server structure.
 * @return uint8_t 1 if command is exit or 0 otherwise.
 */
uint8_t check_if_exit(server_t *this) {
    if (this == NULL) {
        return SERVER_INPUT_IS_NULL;
    }

    if (this->cmd == NULL) {
        return CMD_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    if (this->poll_vec->pfds[0].fd == STDIN_FILENO) {
        if ((this->poll_vec->pfds[0].revents & POLLIN) != 0) {
            if (fgets(this->cmd, MAX_CMD_LEN, stdin) != NULL) {
                return (uint8_t)(strncmp(this->cmd, EXIT_CMD, EXIT_CMD_LEN) == 0);
            }
        }
    }

    return 0;
}
