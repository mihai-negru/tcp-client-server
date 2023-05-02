#ifndef TCP_TYPE_H_
#define TCP_TYPE_H_

#include "./utils.h"

#define MAX_TCP_MSG_BUF_LEN 2048

/**
 * @brief Protocol data structure over the TCP Protocol.
 * 
 */
typedef struct __attribute__((__packed__)) tcp_msg_s {
    uint16_t    len;
    char        data[MAX_TCP_MSG_BUF_LEN];
} tcp_msg_t;

/**
 * @brief Send an exact length message over a tcp socket.
 * 
 * @param tcp_socket socket fd to send the message
 * @param buf message buffer.
 * @param buf_len number of bytes to send over socket.
 * @return err_t OK if buf_len bytes were send over socket
 * or TCP_FAILED_SEND_RECV if connection is closed or sending
 * buf_len bytes failed.
 */
err_t send_tcp_msg(const int tcp_socket, void *buf, size_t buf_len);

/**
 * @brief Receive an exact length message over a tcp socket.
 * 
 * @param tcp_socket socket fd to receive the message
 * @param buf message buffer.
 * @param buf_len number of bytes to receive over socket.
 * @return err_t OK if buf_len bytes were received over socket
 * or TCP_FAILED_SEND_RECV if connection is closed or receiving
 * buf_len bytes failed.
 */
err_t recv_tcp_msg(const int tcp_socket, void *buf, size_t buf_len);

#endif /* TCP_TYPE_H_ */
