#ifndef CLIENT_VEC_H_
#define CLIENT_VEC_H_

#include "./utils.h"

#define REALLOC_FACTOR 2
#define INIT_TOPICS_CAPACITY 10

typedef enum client_stataus_s {
    ACTIVE,
    DEAD
} client_status_t;

typedef enum client_options_s {
    SF,
    NO_SF
} client_options_t;

typedef struct client_type_s {
    char *id;
    client_status_t status;
    client_options_t option;
    int fd;
    char **topics;
    size_t topics_len;
    size_t topic_capacity;
} client_type_t;

typedef struct client_vec_s {
    size_t len;
    size_t capacity;
    client_type_t *entities;
} client_vec_t;

err_t create_clients_vec(client_vec_t **clients, size_t init_clients);
err_t free_clients_vec(client_vec_t **clients);

err_t register_new_client(client_vec_t *clients, char *client_id, int client_fd);
err_t close_active_client(client_vec_t *clients, int client_fd, size_t *client_idx);
char* get_client_id(client_vec_t *clients, size_t client_idx);

err_t subscribe_client_to_topic(client_vec_t *clients, int client_fd, char *client_topic);
err_t unsibscribe_client_from_topic(client_vec_t *clients, int client_fd, char *client_topic);

#endif /* CLIENT_VEC_H_ */