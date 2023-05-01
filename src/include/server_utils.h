#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include "./utils.h"
#include "./poll_vec.h"
#include "./udp_type.h"
#include "./tcp_type.h"
#include "./client_vec.h"

#define MAX_SERVER_BUFLEN 8096
#define MAX_LISTEN_SOCKET 10
#define INIT_NFDS (nfds_t)10
#define INIT_CLIENTS 10
#define INIT_UDP_MSGS_LEN 20
#define EXIT_CMD "exit\0"
#define EXIT_CMD_LEN strlen(EXIT_CMD)

typedef struct server_s {
    int udp_socket;
    int tcp_socket;
    struct sockaddr_in udp_addr;
    struct sockaddr_in tcp_addr;
    poll_vec_t *poll_vec;
    char *buf;
    char *cmd;
    tcp_msg_t *send_msg;
    tcp_msg_t *recv_msg;
    client_vec_t *clients;
    udp_type_t *udp_msgs;
    size_t udp_msgs_len;
    size_t udp_msgs_capacity;
} server_t;

err_t init_server(server_t **server, const uint16_t hport);
err_t free_server(server_t **server);

err_t wait_for_ready_fds(server_t *this);
err_t process_ready_fds(server_t *this);
uint8_t check_if_exit(server_t *this);

#endif /* SERVER_UTILS_H_ */
