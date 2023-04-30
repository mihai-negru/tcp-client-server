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

err_t init_server(server_t **server, const uint16_t hport) {
    if (*server != NULL) {
        return SERVER_INPUT_IS_NOT_NULL;
    }

    if (hport <= 0) {
        return INVALID_PORT_NUMBER;
    }

    *server = malloc(sizeof **server);
    if (*server == NULL) {
        return SERVER_FAILED_ALLOCATION;
    }

    (*server)->buf = malloc(sizeof *(*server)->buf * MAX_SERVER_BUFLEN);
    if ((*server)->buf == NULL) {
        free(*server);
        *server = NULL;

        return SERVER_FAILED_ALLOCATION;
    }

    (*server)->cmd = malloc(sizeof *(*server)->cmd * MAX_CMD_LEN);
    if ((*server)->cmd == NULL) {
        free((*server)->buf);
        free(*server);
        *server = NULL;

        return SERVER_FAILED_ALLOCATION;
    }

    memset((*server)->buf, 0, MAX_SERVER_BUFLEN);
    memset((*server)->cmd, 0, MAX_CMD_LEN);
    
    if (init_server_udp_socket(*server, hport) < 0) {
        free((*server)->buf);
        free((*server)->cmd);
        free(*server);
        *server = NULL;

        return SERVER_FAILED_UDP;
    }

    if (init_server_tcp_socket(*server, hport) < 0) {
        close((*server)->udp_socket);
        free((*server)->buf);
        free((*server)->cmd);
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
        free(*server);
        *server = NULL;

        return SERVER_FAILED_POLL_VEC;
    }
    
    return OK;
}

err_t free_server(server_t **server) {
    if ((server == NULL) || (*server == NULL)) {
        return SERVER_INPUT_IS_NULL;
    }

    close((*server)->udp_socket);
    close((*server)->tcp_socket);

    if ((*server)->poll_vec != NULL) {
        free_poll_vec(&(*server)->poll_vec);
    }

    if ((*server)->buf != NULL) {
        free((*server)->buf);
    }

    if ((*server)->cmd != NULL) {
        free((*server)->cmd);
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
                    if ((err = poll_vec_remove_fd(this->poll_vec, this->poll_vec->pfds[iter].fd)) != OK) {
                        return err;
                    }
                } else {
                    udp_type_t udp_data;
                    memset(&udp_data, 0, sizeof udp_data);

                    parse_udp_type_from(&udp_data, this->buf);
                    print_udp_type(&udp_data);
                }
            }
        }
    }

    return OK;
}