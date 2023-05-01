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

#define KILL(msg)                       \
    do {                                \
        fprintf(stderr, "%s\n", msg);   \
        exit(EXIT_CODE_RED);            \
    } while (0)

#define DEBUG(msg)                       \
    do {                                \
        fprintf(stderr, "%s\n", msg);   \
    } while (0)

#define loop for(;;)

#define MAX_CMD_LEN 10
#define MAX_ID_CLIENT_LEN 10

#define EXIT_CODE_GREEN 0
#define EXIT_CODE_YELLOW -1
#define EXIT_CODE_RED -2

typedef enum err_s {
    OK,
    OK_WITH_EXIT,
    INVALID_PORT_NUMBER,
    POLL_FAILED_TIMED_OUT,
    CMD_INPUT_IS_NULL,

    SERVER_INPUT_IS_NOT_NULL,
    SERVER_INPUT_IS_NULL,
    SERVER_FAILED_ALLOCATION,
    SERVER_FAILED_UDP,
    SERVER_FAILED_TCP,
    SERVER_FAILED_POLL_VEC,
    SERVER_COULD_NOT_CONNECT_NEW_CLIENT,
    SERVER_FAILED_ACCEPT_TCP,
    SERVER_FAILED_CLIENTS_VEC,

    POLL_VEC_INPUT_IS_NOT_NULL,
    POLL_VEC_INPUT_IS_NULL,
    POLL_VEC_FAILED_ALLOCATION,
    POLL_VEC_FAILED_REALLOC,
    POLL_VEC_INPUT_FD_IDX_OUT_OF_BOUND,

    UDP_INPUT_BUF_IS_NULL,
    UDP_INPUT_VAR_IS_NULL,
    UDP_UNKNOWN_DATA_TYPE,

    CLIENT_INPUT_IS_NOT_NULL,
    CLIENT_INPUT_IS_NULL,
    CLIENT_INPUT_CONNECT_IS_NULL,
    CLIENT_FAILED_ALLOCATION,
    CLIENT_FAILED_TCP,
    CLIENT_FAILED_POLL_VEC,

    TCP_INPUT_SOCKET_INVALID,
    TCP_INPUT_BUF_IS_NULL,
    TCP_INPUT_BUF_LEN_IS_ZERO,
    TCP_FAILED_SEND_RECV,

    CLIENTS_VEC_INPUT_IS_NOT_NULL,
    CLIENTS_VEC_FAILED_ALLOCATION,
    CLIENTS_VEC_INPUT_IS_NULL,
    CLIENTS_VEC_FAILED_REALLOC,
    CLIENTS_VEC_FAILED_REGISTER_ALLOCATION,
    CLIENT_VEC_CLIENT_ALREADY_CONNECTED,
    CLIENTS_VEC_INPUT_FD_IDX_INVALID,
    CLIENTS_VEC_CLIENT_ALREADY_DEAD,
    CLIENTS_VEC_CLOSE_CLIENT_NOT_FOUND
} err_t;

void debug_msg(const err_t error);
void debug_msg_and_exit(const err_t error);

uint32_t ipow(uint32_t base, uint8_t exp);

#endif /* UTILS_H_ */
