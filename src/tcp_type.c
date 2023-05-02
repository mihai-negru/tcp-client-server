#include "./include/tcp_type.h"

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
err_t send_tcp_msg(const int tcp_socket, void *buf, size_t buf_len) {
    if (tcp_socket < 0) {
        return TCP_INPUT_SOCKET_INVALID;
    }

    if (buf == NULL) {
        return TCP_INPUT_BUF_IS_NULL;
    }

    if (buf_len == 0) {
        return TCP_INPUT_BUF_LEN_IS_ZERO;
    }

    size_t bytes_send = 0;
    
    ssize_t tcp_bytes = 0;
    while (bytes_send < buf_len) {
        tcp_bytes = send(tcp_socket, (uint8_t *)buf + bytes_send, buf_len - bytes_send, 0);

        if (tcp_bytes <= 0) {
            /* Conection is closed or something wrong happened */

            return TCP_FAILED_SEND_RECV;
        }

        bytes_send += (size_t)tcp_bytes;
    }

    return OK;
}

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
err_t recv_tcp_msg(const int tcp_socket, void *buf, size_t buf_len) {
    if (tcp_socket < 0) {
        return TCP_INPUT_SOCKET_INVALID;
    }

    if (buf == NULL) {
        return TCP_INPUT_BUF_IS_NULL;
    }

    if (buf_len == 0) {
        return TCP_INPUT_BUF_LEN_IS_ZERO;
    }

    size_t bytes_send = 0;
    
    ssize_t tcp_bytes = 0;
    while (bytes_send < buf_len) {
        tcp_bytes = recv(tcp_socket, (uint8_t *)buf + bytes_send, buf_len - bytes_send, 0);

        if (tcp_bytes <= 0) {
            /* Conection is closed or something wrong happened */

            return TCP_FAILED_SEND_RECV;
        }

        bytes_send += (size_t)tcp_bytes;
    }

    return OK;
}
