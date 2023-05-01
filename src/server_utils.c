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
        return subscribe_client_to_topic(this->clients, client_fd, topic, *(uint8_t *)(this->recv_msg->data + this->recv_msg->len - 1));
    } else if (strcmp(cmd, "unsubscribe") == 0) {
        return unsubscribe_client_from_topic(this->clients, client_fd, topic);
    } else {
        return SERVER_UNKNOWN_COMMAND;
    }

    return OK;
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
                    udp_type_t udp_data;
                    memset(&udp_data, 0, sizeof udp_data);

                    parse_udp_type_from(&udp_data, this->buf);
                    print_udp_type(&udp_data);
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
                            printf("Client %s already connected.\n", this->recv_msg->data);
                            poll_vec_remove_fd(this->poll_vec, this->poll_vec->nfds - 1);
                        } else {
                            printf("New client %s connected from %s:%hu.\n", this->recv_msg->data, inet_ntoa(new_client.sin_addr), ntohs(new_client.sin_port));

                            // Here we will have to do the sf actions
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