#ifndef TCP_TYPE_H_
#define TCP_TYPE_H_

#include "./utils.h"

#define MAX_TCP_MSG_BUF_LEN 2048

typedef struct __attribute__((__packed__)) tcp_msg_s {
    uint16_t len;
    char data[MAX_TCP_MSG_BUF_LEN];
} tcp_msg_t;

err_t send_tcp_msg(const int tcp_socket, void *buf, size_t buf_len);
err_t recv_tcp_msg(const int tcp_socket, void *buf, size_t buf_len);

#endif /* TCP_TYPE_H_ */