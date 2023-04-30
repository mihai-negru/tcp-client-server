#ifndef SUBSCRIBER_UTILS_H_
#define SUBSCRIBER_UTILS_H_

#include "./utils.h"
#include "./poll_vec.h"

#define MAX_CLIENT_BUFLEN 8096
#define MAX_CLIENT_CMD_LEN 128
#define MAX_ID_CLIENT_LEN 10

#define SUBSCRIBE_CMD "subscribe\0"
#define SUBSCRIBE_CMD_LEN strlen(SUBSCRIBE_CMD)
#define UNSUBSCRIBE_CMD "unsubscribe\0"
#define UNSUBSCRIBE_CMD_LEN strlen(UNSUBSCRIBE_CMD)
#define EXIT_CMD "exit\0"
#define EXIT_CMD_LEN strlen(EXIT_CMD)

typedef enum cmd_line_s {
    SUBSCRIBE,
    UNSUBSCRIBE,
    EXIT,
    NONE
} cmd_line_t;

typedef struct client_s {
    char *id;
    int tcp_socket;
    struct sockaddr_in tcp_addr;
    poll_vec_t *poll_vec;
    char *buf;
    char *cmd;
} client_t;

err_t init_client(client_t **client, const char *id, const char *ip, const uint16_t hport);
err_t free_client(client_t **client);

err_t wait_for_ready_fds(client_t *this);
err_t process_ready_fds(client_t *this);

err_t process_subscribe_cmd(client_t *this);
err_t process_unsubscribe_cmd(client_t *this);

cmd_line_t get_cmd_line(client_t *this);

#endif /* SUBSCRIBER_UTILS_H_ */
