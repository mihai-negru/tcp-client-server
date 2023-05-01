#include "./include/subscriber_utils.h"

static int init_client_tcp_socket(client_t *client, const char *ip, const uint16_t hport) {
    if ((client->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return client->tcp_socket;
    }

    memset(&client->tcp_addr, 0, sizeof client->tcp_addr);

    client->tcp_addr.sin_family = AF_INET;
    client->tcp_addr.sin_addr.s_addr = inet_addr(ip);
    client->tcp_addr.sin_port = htons(hport);

    if (connect(client->tcp_socket, (const struct sockaddr *) &client->tcp_addr, sizeof client->tcp_addr) < 0) {
        return -1;
    }

    client->send_msg->len = strlen(client->id) + 1;
    strcpy(client->send_msg->data, client->id);
    //memcpy(client->send_msg->data, client->id, client->send_msg->len);
    
    if (send_tcp_msg(client->tcp_socket, (void *)client->send_msg, sizeof *client->send_msg) != OK) {
        return -1;
    }

    return 0;
}

static int init_client_poll_vec(client_t *client) {
    if (create_poll_vec(&client->poll_vec, 2) != OK) {
        return -1;
    }

    if (poll_vec_add_fd(client->poll_vec, STDIN_FILENO, POLLIN) != OK) {
        free_poll_vec(&client->poll_vec);
        return -1;
    }

    if (poll_vec_add_fd(client->poll_vec, client->tcp_socket, POLLIN | POLLOUT) != OK) {
        free_poll_vec(&client->poll_vec);
        return -1;
    }

    return 0;
}

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
    
    memset(client->cmd, 0, MAX_CLIENT_CMD_LEN);
    memset(client->send_msg, 0, sizeof *client->send_msg);
    memset(client->recv_msg, 0, sizeof *client->recv_msg);

    return 0;
}

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
    //memcpy((*client)->id, id, MAX_ID_CLIENT_LEN);

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

err_t wait_for_ready_fds(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    if (poll(this->poll_vec->pfds, this->poll_vec->nfds, -1) <= 0) {
        return POLL_FAILED_TIMED_OUT; 
    }

    return OK;
}

err_t process_ready_fds(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    if (this->poll_vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    err_t err = OK;

    if ((this->poll_vec->pfds[1].revents & POLLIN) != 0) {
        err = recv_tcp_msg(this->tcp_socket, (void *)this->recv_msg, sizeof *this->recv_msg);

        if (err == TCP_FAILED_SEND_RECV) {
            return OK_WITH_EXIT;
        } else if (err != OK) {
            return err;
        } else {
            printf("%s\n", this->recv_msg->data);
        }
    }

    return OK;
}

err_t process_subscribe_cmd(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    char *save_ptr = NULL;
    char *cmd = __strtok_r(this->cmd, " \n", &save_ptr);
    char *topic = __strtok_r(NULL, " \n", &save_ptr);
    uint8_t sf = atoi(__strtok_r(NULL, " \n", &save_ptr));

    size_t send_offset = 0;

    this->send_msg->len = strlen(cmd) + 1;
    memcpy(this->send_msg->data, cmd, this->send_msg->len);

    send_offset += this->send_msg->len;
    this->send_msg->len = strlen(topic) + 1;
    memcpy(this->send_msg->data + send_offset, topic, this->send_msg->len);

    send_offset += this->send_msg->len;
    this->send_msg->len = sizeof sf;
    memcpy(this->send_msg->data + send_offset, &sf, this->send_msg->len);

    this->send_msg->len += send_offset;

    return send_tcp_msg(this->tcp_socket, (void *)this->send_msg, sizeof *this->send_msg);
}

err_t process_unsubscribe_cmd(client_t *this) {
    if (this == NULL) {
        return CLIENT_INPUT_IS_NULL;
    }

    char *save_ptr = NULL;
    char *cmd = __strtok_r(this->cmd, " \n", &save_ptr);
    char *topic = __strtok_r(NULL, " \n", &save_ptr);

    size_t send_offset = 0;

    this->send_msg->len = strlen(cmd) + 1;
    memcpy(this->send_msg->data, cmd, this->send_msg->len);

    send_offset += this->send_msg->len;
    this->send_msg->len = strlen(topic) + 1;
    memcpy(this->send_msg->data + send_offset, topic, this->send_msg->len);

    this->send_msg->len += send_offset;

    return send_tcp_msg(this->tcp_socket, (void *)this->send_msg, sizeof *this->send_msg);
}

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
