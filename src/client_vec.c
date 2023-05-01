#include "./include/client_vec.h"

err_t create_clients_vec(client_vec_t **clients, size_t init_clients) {
    if ((clients == NULL) || (*clients != NULL)) {
        return CLIENTS_VEC_INPUT_IS_NOT_NULL;
    }

    *clients = malloc(sizeof **clients);

    if (*clients == NULL) {
        return CLIENTS_VEC_FAILED_ALLOCATION;
    }

    (*clients)->len = 0;
    (*clients)->capacity = init_clients;
    (*clients)->entities = malloc(sizeof *(*clients)->entities * (*clients)->capacity);

    if ((*clients)->entities == NULL) {
        return CLIENTS_VEC_FAILED_ALLOCATION;
    }

    return OK;
}

err_t free_clients_vec(client_vec_t **clients) {
    if ((clients == NULL) || (*clients == NULL)) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    for (size_t iter = 0; iter < (*clients)->len; ++iter) {
        for (size_t iter_j = 0; iter_j < (*clients)->entities[iter].topics_len; ++iter_j) {
            free((*clients)->entities[iter].topics[iter_j]);
        }

        free((*clients)->entities[iter].topics);
        free((*clients)->entities[iter].id);
        free((*clients)->entities[iter].options);
        free((*clients)->entities[iter].ready_msgs);
    }

    if ((*clients)->entities != NULL) {
        free((*clients)->entities);
    }

    free(*clients);
    *clients = NULL;

    return OK;
}

err_t register_new_client(client_vec_t *clients, char *client_id, int client_fd) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    for (size_t iter = 0; iter < clients->len; ++iter) {
        if (strcmp(clients->entities[iter].id, client_id) == 0) {
            if (clients->entities[iter].status == ACTIVE) {
                return CLIENT_VEC_CLIENT_ALREADY_CONNECTED;
            }
            
            clients->entities[iter].fd = client_fd;
            clients->entities[iter].status = ACTIVE;

            return OK;
        }
    }

    if (clients->len == clients->capacity) {
        client_type_t *entities_real = realloc(clients->entities, sizeof *clients->entities * clients->capacity * REALLOC_FACTOR);

        if (entities_real == NULL) {
            return CLIENTS_VEC_FAILED_REALLOC;
        }

        clients->entities = entities_real;
        clients->capacity *= REALLOC_FACTOR;
    }

    clients->entities[clients->len].id = malloc(MAX_ID_CLIENT_LEN);

    if (clients->entities[clients->len].id == NULL) {
        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    clients->entities[clients->len].topics_len = 0;
    clients->entities[clients->len].topic_capacity = INIT_TOPICS_CAPACITY;\

    clients->entities[clients->len].topics = malloc(sizeof *clients->entities[clients->len].topics * INIT_TOPICS_CAPACITY);
    if (clients->entities[clients->len].topics == NULL) {
        free(clients->entities[clients->len].id);

        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    clients->entities[clients->len].options = malloc(sizeof *clients->entities[clients->len].options * INIT_TOPICS_CAPACITY);
    if (clients->entities[clients->len].options == NULL) {
        free(clients->entities[clients->len].id);
        free(clients->entities[clients->len].topics);

        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    clients->entities[clients->len].ready_msgs_len = 0;
    clients->entities[clients->len].ready_msgs_capacity = INIT_TOPICS_CAPACITY;

    clients->entities[clients->len].ready_msgs = malloc(sizeof *clients->entities[clients->len].ready_msgs * INIT_TOPICS_CAPACITY);
    if (clients->entities[clients->len].ready_msgs == NULL) {
        free(clients->entities[clients->len].id);
        free(clients->entities[clients->len].topics);
        free(clients->entities[clients->len].options);

        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    strcpy(clients->entities[clients->len].id, client_id);

    clients->entities[clients->len].fd = client_fd;
    clients->entities[clients->len].status = ACTIVE;

    (clients->len)++;

    return OK;
}

err_t close_active_client(client_vec_t *clients, int client_fd, size_t *client_idx) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    if (client_fd < 0) {
        return CLIENTS_VEC_INPUT_FD_IDX_INVALID;
    }

    for (size_t iter = 0; iter < clients->len; ++iter) {
        if (clients->entities[iter].fd == client_fd) {
            if (clients->entities[iter].status == DEAD) {
                return CLIENTS_VEC_CLIENT_ALREADY_DEAD;
            }

            clients->entities[iter].status = DEAD;
            clients->entities[iter].fd = -1;
            *client_idx = iter;

            return OK;
        }
    }

    return CLIENTS_VEC_CLOSE_CLIENT_NOT_FOUND;
}

char* get_client_id(client_vec_t *clients, size_t client_idx) {
    return clients->entities[client_idx].id;
}

err_t subscribe_client_to_topic(client_vec_t *clients, int client_fd, char *client_topic, uint8_t client_sf) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    if (client_fd < 0) {
        return CLIENTS_VEC_INPUT_FD_IDX_INVALID;
    }

    for (size_t iter = 0; iter < clients->len; ++iter) {
        if (clients->entities[iter].fd == client_fd) {
            if (clients->entities[iter].topics_len == clients->entities[iter].topic_capacity) {
                char **topics_real = realloc(clients->entities[iter].topics, sizeof * clients->entities[iter].topics * clients->entities[iter].topic_capacity * REALLOC_FACTOR);

                if (topics_real == NULL) {
                    return CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC;
                }

                clients->entities[iter].topics = topics_real;

                client_options_t *options_real = realloc(clients->entities[iter].options, sizeof * clients->entities[iter].options * clients->entities[iter].topic_capacity * REALLOC_FACTOR);

                if (options_real == NULL) {
                    return CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC;
                }

                clients->entities[iter].options = options_real;

                clients->entities[iter].topic_capacity *= REALLOC_FACTOR;
            }

            clients->entities[iter].topics[clients->entities[iter].topics_len] = malloc(strlen(client_topic) + 1);

            if (clients->entities[iter].topics[clients->entities[iter].topics_len] == NULL) {
                return CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC;
            }

            memcpy(clients->entities[iter].topics[clients->entities[iter].topics_len], client_topic, strlen(client_topic) + 1);
            clients->entities[iter].options[clients->entities[iter].topics_len] = client_sf;

            (clients->entities[iter].topics_len)++;

            return OK;
        }
    }

    return CLIENTS_VEC_CLIENT_NOT_FOUND;
}

err_t unsubscribe_client_from_topic(client_vec_t *clients, int client_fd, char *client_topic) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    if (client_fd < 0) {
        return CLIENTS_VEC_INPUT_FD_IDX_INVALID;
    }

    for (size_t iter = 0; iter < clients->len; ++iter) {
        if (clients->entities[iter].fd == client_fd) {
            for (size_t iter_j = 0; iter_j < clients->entities[iter].topics_len; ++iter_j) {
                if (strcmp(clients->entities[iter].topics[iter_j], client_topic) == 0) {
                    free(clients->entities[iter].topics[iter_j]);
                    
                    for (; iter_j < clients->entities[iter].topics_len - 1; ++iter_j) {
                        clients->entities[iter].topics[iter_j] = clients->entities[iter].topics[iter_j + 1];
                        clients->entities[iter].options[iter_j] = clients->entities[iter].options[iter_j + 1];
                    }

                    (clients->entities[iter].topics_len)--;

                    return OK;
                }
            }

            return CLIENTS_VEC_COUND_NOT_FIND_TOPIC;
        }
    }

    return CLIENTS_VEC_CLIENT_NOT_FOUND;
}

err_t add_topic_msg_for_client(client_vec_t *clients, udp_type_t *udp_msg, size_t client_idx) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    if (client_idx >= clients->len) {
        return CLIENTS_VEC_INDEX_OUT_OF_BOUND;
    }

    if (clients->entities[client_idx].ready_msgs_len == clients->entities[client_idx].ready_msgs_capacity) {
        udp_type_t **ready_msgs_real = realloc(
            clients->entities[client_idx].ready_msgs,
            sizeof *clients->entities[client_idx].ready_msgs * clients->entities[client_idx].ready_msgs_capacity * REALLOC_FACTOR);

        if (ready_msgs_real == NULL) {
            return CLIENTS_VEC_FAILED_REALLOC;
        }

        clients->entities[client_idx].ready_msgs = ready_msgs_real;
        clients->entities[client_idx].ready_msgs_capacity *= REALLOC_FACTOR;
    }

    clients->entities[client_idx].ready_msgs[clients->entities[client_idx].ready_msgs_len] = udp_msg;
    (clients->entities[client_idx].ready_msgs_len)++;

    return OK;
}