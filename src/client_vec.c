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
        if (strncmp(clients->entities[iter].id, client_id, MAX_ID_CLIENT_LEN) == 0) {
            if (clients->entities[iter].status == ACTIVE) {
                return CLIENT_VEC_CLIENT_ALREADY_CONNECTED;
            }
            
            clients->entities[clients->len].fd = client_fd;
            clients->entities[clients->len].status = ACTIVE;

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
    clients->entities[clients->len].topic_capacity = INIT_TOPICS_CAPACITY;
    clients->entities[clients->len].topics = malloc(sizeof *clients->entities[clients->len].topics * INIT_TOPICS_CAPACITY);

    if (clients->entities[clients->len].topics == NULL) {
        free(clients->entities[clients->len].id);

        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    strncpy(clients->entities[clients->len].id, client_id, MAX_ID_CLIENT_LEN - 1);

    clients->entities[clients->len].fd = client_fd;
    clients->entities[clients->len].status = ACTIVE;
    clients->entities[clients->len].option = NO_SF;

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

err_t subscribe_client_to_topic(client_vec_t *clients, int client_fd, char *client_topic) {
    (void)clients;
    (void)client_fd;
    (void)client_topic;
    return OK;
}

err_t unsibscribe_client_from_topic(client_vec_t *clients, int client_fd, char *client_topic) {
    (void)clients;
    (void)client_fd;
    (void)client_topic;
    return OK;
}