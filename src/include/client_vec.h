/**
 * @file client_vec.h
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

#ifndef CLIENT_VEC_H_
#define CLIENT_VEC_H_

#include "./utils.h"
#include "./udp_type.h"

#define INIT_TOPICS_CAPACITY 10

/**
 * @brief Enum type class in order to
 * handle the status of a client.
 * 
 */
typedef enum client_stataus_s {
    ACTIVE  = 0,
    DEAD    = 1
} client_status_t;

/**
 * @brief Enum type class in order to
 * handle Store and Forward functionality
 * of a client.
 * 
 */
typedef enum client_options_s {
    NO_SF   = 0,
    SF      = 1
} client_options_t;

/**
 * @brief Structure type class to encode
 * client's data and metadata.
 * 
 */
typedef struct client_type_s {
    char                *id;                    /* Unique client ID */
    client_status_t     status;
    client_options_t    *options;               /* Store and forward options */
    int                 fd;                     /* Open socket file descriptor */
    char                **topics;               /* Subscribed topic names */
    size_t              topics_len;
    size_t              topic_capacity;
    udp_type_t          **ready_msgs;           /* Unsent UDP messages ready to be sent */
    size_t              ready_msgs_len;
    size_t              ready_msgs_capacity;
} client_type_t;

typedef struct client_vec_s {
    size_t          len;
    size_t          capacity;
    client_type_t   *entities;
} client_vec_t;

/**
 * @brief Creates a clients vec object for clients data and metadata.
 * 
 * @param clients pointer to vector structure to allocate must be NULL.
 * @param init_clients initial vector length to allocate.
 * @return err_t OK if vector could be allocated, error otherwise.
 */
err_t create_clients_vec(client_vec_t **clients, size_t init_clients);

/**
 * @brief Frees a clients vector object
 * and sets the vector to NULL.
 * 
 * @param clients pointer to vector structure. 
 * @return err_t OK if vector was freed, error otherwise.
 */
err_t free_clients_vec(client_vec_t **clients);

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
err_t register_new_client(client_vec_t *clients, char *client_id, int client_fd);

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
err_t close_active_client(client_vec_t *clients, int client_fd, size_t *client_idx);

/**
 * @brief Gets the client's id. The id can be accessed
 * even if the client is DEAD.
 * 
 * @param clients clients vector structure.
 * @param client_idx valid client index.
 * @return char* ID if client was found or `""` if
 * client's id could not be fetched.
 */
const char* get_client_id(client_vec_t *clients, size_t client_idx);

/**
 * @brief Adds a new topic and a new option for the selected topic
 * for the specified client, the client is found over its valid socket
 * file descriptor.
 * 
 * @param clients clients vector structure.
 * @param client_fd valid socket file descriptor assigned for the client.
 * @param client_topic string topic name to add.
 * @param client_sf enum option class for the specified topic name.
 * @return err_t OK if the topic was addded successfully or error otherwise.
 */
err_t subscribe_client_to_topic(client_vec_t *clients, int client_fd, char *client_topic, client_options_t client_sf);

/**
 * @brief Removes a topic and it's option for the specified client, the
 * client is found over its valid socket file descriptor.
 * 
 * @param clients clients vector structure.
 * @param client_fd valid socket file descriptor assigned for the client.
 * @param client_topic string topic name to remove
 * @return err_t OK if the topic was removed successfully or error otherwise.
 */
err_t unsubscribe_client_from_topic(client_vec_t *clients, int client_fd, char *client_topic);

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
err_t add_topic_msg_for_client(client_vec_t *clients, udp_type_t *udp_msg, size_t client_idx);

#endif /* CLIENT_VEC_H_ */
