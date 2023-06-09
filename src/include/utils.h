/**
 * @file utils.h
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

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <unistd.h>
#include <poll.h>

/**
 * @brief Macro to print a debug message
 * and to exit with EXIT_COD_RED the current
 * thread.
 *
 */
#define KILL(msg)                       \
    do {                                \
        fprintf(stderr, "%s\n", msg);   \
        exit(EXIT_CODE_RED);            \
    } while (0)

/**
 * @brief Macro to print a debug message
 * without exiting the current thread.
 *
 */
#define DEBUG(msg)                      \
    do {                                \
        fprintf(stderr, "%s\n", msg);   \
    } while (0)

#define loop for(;;)

#define REALLOC_FACTOR      2

#define MAX_CMD_LEN         10
#define MAX_ID_CLIENT_LEN   10

#define EXIT_CODE_GREEN     0
#define EXIT_CODE_YELLOW    -1
#define EXIT_CODE_RED       -2

/**
 * @brief Error type class to handle
 * the errors.
 *
 * The class is build over the socket's
 * API errors and other errors.
 *
 */
typedef enum err_s {
    OK                                          = 0,
    OK_WITH_EXIT                                = -1,
    INVALID_PORT_NUMBER                         = -2,
    POLL_FAILED_TIMED_OUT                       = -3,
    CMD_INPUT_IS_NULL                           = -4,

    SERVER_INPUT_IS_NOT_NULL                    = -5,
    SERVER_INPUT_IS_NULL                        = -6,
    SERVER_FAILED_ALLOCATION                    = -7,
    SERVER_FAILED_UDP                           = -8,
    SERVER_FAILED_TCP                           = -9,
    SERVER_FAILED_POLL_VEC                      = -10,
    SERVER_COULD_NOT_CONNECT_NEW_CLIENT         = -11,
    SERVER_FAILED_ACCEPT_TCP                    = -12,
    SERVER_FAILED_CLIENTS_VEC                   = -13,
    SERVER_UNKNOWN_COMMAND                      = -14,
    SERVER_COULD_NOT_ADD_NEW_UDP                = -15,

    POLL_VEC_INPUT_IS_NOT_NULL                  = -16,
    POLL_VEC_INPUT_IS_NULL                      = -17,
    POLL_VEC_FAILED_ALLOCATION                  = -18,
    POLL_VEC_FAILED_REALLOC                     = -19,
    POLL_VEC_INPUT_FD_IDX_OUT_OF_BOUND          = -20,

    UDP_INPUT_BUF_IS_NULL                       = -21,
    UDP_INPUT_VAR_IS_NULL                       = -22,
    UDP_UNKNOWN_DATA_TYPE                       = -23,

    CLIENT_INPUT_IS_NOT_NULL                    = -24,
    CLIENT_INPUT_IS_NULL                        = -25,
    CLIENT_INPUT_CONNECT_IS_NULL                = -26,
    CLIENT_FAILED_ALLOCATION                    = -27,
    CLIENT_FAILED_TCP                           = -28,
    CLIENT_FAILED_POLL_VEC                      = -29,

    TCP_INPUT_SOCKET_INVALID                    = -30,
    TCP_INPUT_BUF_IS_NULL                       = -31,
    TCP_INPUT_BUF_LEN_IS_ZERO                   = -32,
    TCP_FAILED_SEND_RECV                        = -33,

    CLIENTS_VEC_INPUT_IS_NOT_NULL               = -34,
    CLIENTS_VEC_FAILED_ALLOCATION               = -35,
    CLIENTS_VEC_INPUT_IS_NULL                   = -36,
    CLIENTS_VEC_FAILED_REALLOC                  = -37,
    CLIENTS_VEC_FAILED_REGISTER_ALLOCATION      = -38,
    CLIENT_VEC_CLIENT_ALREADY_CONNECTED         = -39,
    CLIENTS_VEC_INPUT_FD_IDX_INVALID            = -40,
    CLIENTS_VEC_CLIENT_ALREADY_DEAD             = -41,
    CLIENTS_VEC_CLOSE_CLIENT_NOT_FOUND          = -42,
    CLIENTS_VEC_CLIENT_NOT_FOUND                = -43,
    CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC           = -44,
    CLIENTS_VEC_COUND_NOT_FIND_TOPIC            = -45,
    CLIENTS_VEC_INDEX_OUT_OF_BOUND              = -46,

    INPUT_WRONG_FORMAT                          = -47
} err_t;

/**
 * @brief Print the current error message
 * in a friendly format on stderr.
 *
 * @param error error type class generated by a function.
 */
void debug_msg(const err_t error);

/**
 * @brief Print the current error message
 * in a friendly format on stderr and exit
 * the current thread with EXIT_CODE_RED.
 *
 * @param error error type class generated by a function.
 */
void debug_msg_and_exit(const err_t error);

/**
 * @brief Function to calculate the pow(base, exp)
 * in logarithmic time over integer base and exponential
 * values.
 *
 * @param base the base number.
 * @param exp the exponent number.
 * @return uint32_t base ^ exp.
 */
uint32_t ipow(uint32_t base, uint8_t exp);

#endif /* UTILS_H_ */
