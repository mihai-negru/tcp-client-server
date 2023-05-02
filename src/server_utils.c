#include "./include/server_utils.h"

static int init_server_udp_socket(server_t *server, const uint16_t hport) {
    if ((server->udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        return server->udp_socket;
    }

    memset(&server->udp_addr, 0, sizeof server->udp_addr);

    server->udp_addr.sin_family = AF_INET;
    server->udp_addr.sin_addr.s_addr = INADDR_ANY;
    server->udp_addr.sin_port = htons(hport);

    if (bind(server->udp_socket, (const struct sockaddr *) &server->udp_addr, sizeof server->udp_addr) < 0) {
        return -1;
    }

    return 0;
}

static int init_server_tcp_socket(server_t *server, const uint16_t hport) {
    if ((server->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return server->tcp_socket;
    }

    memset(&server->tcp_addr, 0, sizeof server->tcp_addr);

    server->tcp_addr.sin_family = AF_INET;
    server->tcp_addr.sin_addr.s_addr = INADDR_ANY;
    server->tcp_addr.sin_port = htons(hport);

    if (bind(server->tcp_socket, (const struct sockaddr *) &server->tcp_addr, sizeof server->tcp_addr) < 0) {
        return -1;
    }

    if (listen(server->tcp_socket, MAX_LISTEN_SOCKET) < 0) {
        return -1;
    }

    if (setsockopt(server->tcp_socket, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof (int)) < 0) {
        return -1;
    }

    return 0;
}

static int init_server_poll_vec(server_t *server) {
    if (create_poll_vec(&server->poll_vec, INIT_NFDS) != OK) {
        return -1;
    }

    if (poll_vec_add_fd(server->poll_vec, STDIN_FILENO, POLLIN) != OK) {
        free_poll_vec(&server->poll_vec);
        return -1;
    }

    if (poll_vec_add_fd(server->poll_vec, server->udp_socket, POLLIN) != OK) {
        free_poll_vec(&server->poll_vec);
        return -1;
    }

    if (poll_vec_add_fd(server->poll_vec, server->tcp_socket, POLLIN) != OK) {
        free_poll_vec(&server->poll_vec);
        return -1;
    }

    return 0;
}

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
    

    memset(server->buf, 0, MAX_SERVER_BUFLEN);
    memset(server->cmd, 0, MAX_CMD_LEN);
    memset(server->send_msg, 0, sizeof *server->send_msg);
    memset(server->recv_msg, 0, sizeof *server->recv_msg);

    return 0;
}

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

err_t wait_for_ready_fds(server_t *this) {
    if (this == NULL) {
        return SERVER_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    if (poll(this->poll_vec->pfds, this->poll_vec->nfds, -1) <= 0) {
        return POLL_FAILED_TIMED_OUT; 
    }

    return OK;
}

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

static err_t process_server_tcp_msg(server_t *this, int client_fd) {
    char *cmd = this->recv_msg->data;
    char *topic = this->recv_msg->data + strlen(cmd) + 1;

    if (strcmp(cmd, "subscribe") == 0) {
        return subscribe_client_to_topic(this->clients, client_fd, topic, *(uint8_t *)(this->recv_msg->data + this->recv_msg->len - 1) == 0 ? NO_SF : SF);
    } else if (strcmp(cmd, "unsubscribe") == 0) {
        return unsubscribe_client_from_topic(this->clients, client_fd, topic);
    } else {
        return SERVER_UNKNOWN_COMMAND;
    }

    return OK;
}

static err_t add_server_udp_msg(server_t *this) {
    err_t err = OK;

    if (this->udp_msgs_len == this->udp_msgs_capacity) {
        udp_type_t *udp_msgs_real = realloc(this->udp_msgs, sizeof *this->udp_msgs * this->udp_msgs_capacity * REALLOC_FACTOR);

        if (udp_msgs_real == NULL) {
            return SERVER_COULD_NOT_ADD_NEW_UDP;
        }

        this->udp_msgs = udp_msgs_real;
        this->udp_msgs_capacity *= REALLOC_FACTOR;
    }

    memset(&this->udp_msgs[this->udp_msgs_len], 0, sizeof *this->udp_msgs);
    if ((err = parse_udp_type_from(&this->udp_msgs[this->udp_msgs_len], this->buf)) != OK) {
        return err;
    }

    (this->udp_msgs_len)++;

    return OK;
}

static err_t pack_topic_to_tcp_msg(server_t *this, udp_type_t *msg) {
    this->send_msg->len = snprintf(this->send_msg->data, MAX_TCP_MSG_BUF_LEN, "%s -", msg->topic);

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

static err_t transmit_topic_to_clients(server_t *this) {
    err_t err = OK;

    if ((err = pack_topic_to_tcp_msg(this, &this->udp_msgs[this->udp_msgs_len - 1]))) {
        return err;
    }

    for (size_t iter = 0; iter < this->clients->len; ++iter) {
        for (size_t iter_j = 0; iter_j < this->clients->entities[iter].topics_len; ++iter_j) {
            if (strcmp(this->udp_msgs[this->udp_msgs_len - 1].topic, this->clients->entities[iter].topics[iter_j]) == 0) {
                if (this->clients->entities[iter].status == ACTIVE) {
                    if ((err = send_tcp_msg(this->clients->entities[iter].fd, (void *)this->send_msg, sizeof *this->send_msg)) != OK) {
                        debug_msg(err);
                    }
                } else if (this->clients->entities[iter].options[iter_j] == SF) {
                    if ((err = add_topic_msg_for_client(this->clients, &this->udp_msgs[this->udp_msgs_len - 1], iter)) != OK) {
                        return err;
                    }
                }

                break;
            }
        }
    }

    return OK;
}

static err_t retransmit_topics_to_client(server_t *this, int client_fd) {
    err_t err = OK;

    for (size_t iter = 0; iter < this->clients->len; ++iter) {
        if (this->clients->entities[iter].fd == client_fd) {
            size_t remaining_msgs = this->clients->entities[iter].ready_msgs_len;
            
            for (; remaining_msgs > 0; remaining_msgs--) {
                if ((err = pack_topic_to_tcp_msg(this, this->clients->entities[iter].ready_msgs[remaining_msgs - 1]))) {
                    this->clients->entities[iter].ready_msgs_len = remaining_msgs;
                    return err;
                }

                if ((err = send_tcp_msg(client_fd, (void *)this->send_msg, sizeof *this->send_msg)) != OK) {
                    this->clients->entities[iter].ready_msgs_len = remaining_msgs;
                    debug_msg(err);
                    
                    break;
                }
            }

            this->clients->entities[iter].ready_msgs_len = 0;
            
            return OK;
        }
    }

    return CLIENTS_VEC_CLIENT_NOT_FOUND;
}

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
                ssize_t udp_bytes = recv(this->udp_socket, this->buf, MAX_SERVER_BUFLEN, 0);

                if (udp_bytes == 0) {
                    if ((err = poll_vec_remove_fd(this->poll_vec, iter)) != OK) {
                        return err;
                    }

                    iter--;
                } else {
                    if ((err = add_server_udp_msg(this)) != OK) {
                        return err;
                    }

                    if ((err = transmit_topic_to_clients(this)) != OK) {
                        return err;
                    }
                }
            } else if (this->poll_vec->pfds[iter].fd == this->tcp_socket) {
                struct sockaddr_in new_client;
                memset(&new_client, 0, sizeof new_client);

                int new_client_fd = accept(this->tcp_socket, (struct sockaddr *) &new_client, &(socklen_t){sizeof new_client});

                if (new_client_fd <= 0) {
                    return SERVER_FAILED_ACCEPT_TCP;
                } else {
                    if ((err = poll_vec_add_fd(this->poll_vec, new_client_fd, POLLIN | POLLOUT)) != OK) {
                        return err;
                    }

                    if ((err = recv_tcp_msg(new_client_fd, (void *)this->recv_msg, sizeof *this->recv_msg)) != OK) {
                        return SERVER_COULD_NOT_CONNECT_NEW_CLIENT;
                    } else {
                        if ((err = register_new_client(this->clients, this->recv_msg->data, new_client_fd)) != OK) {
                            poll_vec_remove_fd(this->poll_vec, this->poll_vec->nfds - 1);
                            printf("Client %s already connected.\n", this->recv_msg->data);
                        } else {
                            printf("New client %s connected from %s:%hu.\n", this->recv_msg->data, inet_ntoa(new_client.sin_addr), ntohs(new_client.sin_port));

                            if ((err = retransmit_topics_to_client(this, new_client_fd)) != OK) {
                                debug_msg(err);
                            }
                        }
                    }
                }
            } else {
                if ((err = recv_tcp_msg(this->poll_vec->pfds[iter].fd, (void *)this->recv_msg, sizeof *this->recv_msg)) != OK) {
                    size_t close_client_idx = 0;
                    if ((err = close_active_client(this->clients, this->poll_vec->pfds[iter].fd, &close_client_idx)) != OK) {
                        debug_msg(err);
                    } else {
                        poll_vec_remove_fd(this->poll_vec, iter);
                        iter--;
                        printf("Client %s disconnected.\n", get_client_id(this->clients, close_client_idx));
                    }
                } else {
                    if ((err = process_server_tcp_msg(this, this->poll_vec->pfds[iter].fd)) != OK) {
                        debug_msg(err);
                    }
                }
            }
        }
    }

    return OK;
}