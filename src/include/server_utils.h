#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include "./utils.h"
#include "./poll_vec.h"

#define MAX_SERVER_BUFLEN 8096
#define MAX_LISTEN_SOCKET 10
#define INIT_NFDS (nfds_t)10
#define EXIT_CMD "exit\0"

typedef struct server_s {
    int udp_socket;
    int tcp_socket;
    struct sockaddr_in udp_addr;
    struct sockaddr_in tcp_addr;
    poll_vec_t *poll_vec;
    char *buf;
} server_t;

err_t init_server(server_t **server, const uint16_t hport);
err_t free_server(server_t **server);

#endif /* SERVER_UTILS_H_ */
