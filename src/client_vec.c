/**
 * @file client_vec.c
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

#include "./include/client_vec.h"

/**
 * @brief Creates a clients vec object for clients data and metadata.
 *
 * @param clients pointer to vector structure to allocate must be NULL.
 * @param init_clients initial vector length to allocate.
 * @return err_t OK if vector could be allocated, error otherwise.
 */
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

/**
 * @brief Frees a clients vector object
 * and sets the vector to NULL.
 *
 * @param clients pointer to vector structure.
 * @return err_t OK if vector was freed, error otherwise.
 */
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

/**
 * @brief Connect a client to the server, if the new client's
 * ID is found if another client with the same id is active,
 * the function will return with an error, else if the client
 * is dead the function returns with OK and a new socket file
 * descripot is assigned again for the client in the structure,
 * else a new client is generated and unction returns with OK.
 *
 * @param clients clients vector structure.
 * @param client_id id of the client to register.
 * @param client_fd valid socket file descripor assigned for the client.
 * @return err_t OK if client was registered successfully or error otherwise.
 */
err_t register_new_client(client_vec_t *clients, char *client_id, int client_fd) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    /* Checks if client was ever registered */
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

    /* Adds memory for new clients */
    if (clients->len == clients->capacity) {
        client_type_t *entities_real = realloc(
            clients->entities,
            sizeof *clients->entities * clients->capacity * REALLOC_FACTOR
        );

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

    clients->entities[clients->len].topics =
        malloc(sizeof *clients->entities[clients->len].topics * INIT_TOPICS_CAPACITY);
    if (clients->entities[clients->len].topics == NULL) {
        free(clients->entities[clients->len].id);

        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    clients->entities[clients->len].options =
        malloc(sizeof *clients->entities[clients->len].options * INIT_TOPICS_CAPACITY);
    if (clients->entities[clients->len].options == NULL) {
        free(clients->entities[clients->len].id);
        free(clients->entities[clients->len].topics);

        return CLIENTS_VEC_FAILED_REGISTER_ALLOCATION;
    }

    clients->entities[clients->len].ready_msgs_len = 0;
    clients->entities[clients->len].ready_msgs_capacity = INIT_TOPICS_CAPACITY;

    clients->entities[clients->len].ready_msgs =
        malloc(sizeof *clients->entities[clients->len].ready_msgs * INIT_TOPICS_CAPACITY);
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

/**
 * @brief Assigns to a client a DEAD status and sets the socket file
 * descriptor to -1. Does NOT free the memory assigned for a client, because
 * the client can reconnect witht he same ID.
 *
 * @param clients clients vector structure.
 * @param client_fd valid socket file descriptor in order to find the client.
 * @param client_idx pointer to variable to set the index of the closed client.
 * @return err_t OK if closing the client went successfully or error otherwise.
 */
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

/**
 * @brief Gets the client's id. The id can be accessed
 * even if the client is DEAD.
 *
 * @param clients clients vector structure.
 * @param client_idx valid client index.
 * @return char* ID if client was found or `""` if
 * client's id could not be fetched.
 */
const char* get_client_id(client_vec_t *clients, size_t client_idx) {
    if ((clients == NULL) || (client_idx >= clients->len)) {
        return "";
    }

    return clients->entities[client_idx].id;
}

/**
 * @brief Adds a new topic and a new option for the selected topic
 * for the specified client, the client is found over its valid socket
 * file descriptor. If the topic already exists the option will be updated
 *
 * @param clients clients vector structure.
 * @param client_fd valid socket file descriptor assigned for the client.
 * @param client_topic string topic name to add.
 * @param client_sf enum option class for the specified topic name.
 * @return err_t OK if the topic was addded successfully or error otherwise.
 */
err_t subscribe_client_to_topic(client_vec_t *clients, int client_fd,
    char *client_topic, client_options_t client_sf) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    if (client_fd < 0) {
        return CLIENTS_VEC_INPUT_FD_IDX_INVALID;
    }

    for (size_t iter = 0; iter < clients->len; ++iter) {
        if (clients->entities[iter].fd == client_fd) {

            /* Check if the topic already exists, if yes update the options */
            for (size_t iter_j = 0; iter_j < clients->entities[iter].topics_len; ++iter_j) {
                if (strcmp(clients->entities[iter].topics[iter_j], client_topic) == 0) {
                    clients->entities[iter].options[iter_j] = client_sf;

                    return OK;
                }
            }

            /* Adds more memory for new topics */
            if (clients->entities[iter].topics_len == clients->entities[iter].topic_capacity) {
                char **topics_real = realloc(
                    clients->entities[iter].topics,
                    sizeof * clients->entities[iter].topics *
                    clients->entities[iter].topic_capacity * REALLOC_FACTOR
                );

                if (topics_real == NULL) {
                    return CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC;
                }

                clients->entities[iter].topics = topics_real;

                client_options_t *options_real = realloc(
                    clients->entities[iter].options,
                    sizeof * clients->entities[iter].options *
                    clients->entities[iter].topic_capacity * REALLOC_FACTOR
                );

                if (options_real == NULL) {
                    return CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC;
                }

                clients->entities[iter].options = options_real;
                clients->entities[iter].topic_capacity *= REALLOC_FACTOR;
            }

            clients->entities[iter].topics[clients->entities[iter].topics_len] =
                malloc(strlen(client_topic) + 1);

            if (clients->entities[iter].topics[clients->entities[iter].topics_len] == NULL) {
                return CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC;
            }

            strcpy(
                clients->entities[iter].topics[clients->entities[iter].topics_len],
                client_topic
            );

            clients->entities[iter].options[clients->entities[iter].topics_len] = client_sf;

            (clients->entities[iter].topics_len)++;

            return OK;
        }
    }

    return CLIENTS_VEC_CLIENT_NOT_FOUND;
}

/**
 * @brief Removes a topic and it's option for the specified client, the
 * client is found over its valid socket file descriptor.
 *
 * @param clients clients vector structure.
 * @param client_fd valid socket file descriptor assigned for the client.
 * @param client_topic string topic name to remove
 * @return err_t OK if the topic was removed successfully or error otherwise.
 */
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
                    /* Remove topic from client metadata */

                    free(clients->entities[iter].topics[iter_j]);

                    for (; iter_j < clients->entities[iter].topics_len - 1; ++iter_j) {
                        clients->entities[iter].topics[iter_j] =
                            clients->entities[iter].topics[iter_j + 1];
                        clients->entities[iter].options[iter_j] =
                            clients->entities[iter].options[iter_j + 1];
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

/**
 * @brief Stackes a UDP message for a client. The client
 * stores unsent messages opon Store and Forward functionality.
 *
 * The client is found over his index to generate a O(1) search
 * for fast time response.
 *
 * @param clients clients vector structure.
 * @param udp_msg UDP message to store in the queue.
 * @param client_idx valid client index in order to find the client.
 * @return err_t OK if the message wasa stacked successfully or error otherwsie
 */
err_t add_topic_msg_for_client(client_vec_t *clients, udp_type_t *udp_msg, size_t client_idx) {
    if (clients == NULL) {
        return CLIENTS_VEC_INPUT_IS_NULL;
    }

    if (client_idx >= clients->len) {
        return CLIENTS_VEC_INDEX_OUT_OF_BOUND;
    }

    /* Adds more memory for the stacked udp messages */
    if (clients->entities[client_idx].ready_msgs_len ==
        clients->entities[client_idx].ready_msgs_capacity) {

        udp_type_t **ready_msgs_real = realloc(
            clients->entities[client_idx].ready_msgs,
            sizeof *clients->entities[client_idx].ready_msgs *
            clients->entities[client_idx].ready_msgs_capacity * REALLOC_FACTOR
        );

        if (ready_msgs_real == NULL) {
            return CLIENTS_VEC_FAILED_REALLOC;
        }

        clients->entities[client_idx].ready_msgs = ready_msgs_real;
        clients->entities[client_idx].ready_msgs_capacity *= REALLOC_FACTOR;
    }

    clients->entities[client_idx]
        .ready_msgs[clients->entities[client_idx].ready_msgs_len] = udp_msg;
    (clients->entities[client_idx].ready_msgs_len)++;

    return OK;
}
