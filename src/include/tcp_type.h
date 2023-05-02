/**
 * @file tcp_type.h
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
