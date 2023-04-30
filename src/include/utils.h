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
#define EXIT_CODE_GREEN 0
#define EXIT_CODE_YELLOW -1
#define EXIT_CODE_RED -2

typedef enum err_s {
    OK,
    OK_WITH_EXIT,

    SERVER_INPUT_IS_NOT_NULL,
    SERVER_INPUT_IS_NULL,
    INVALID_PORT_NUMBER,
    SERVER_FAILED_ALLOCATION,
    SERVER_FAILED_UDP,
    SERVER_FAILED_TCP,
    SERVER_FAILED_POLL_VEC,

    POLL_VEC_INPUT_IS_NOT_NULL,
    POLL_VEC_INPUT_IS_NULL,
    POLL_VEC_FAILED_ALLOCATION,
    POLL_VEC_FAILED_REALLOC,
    POLL_VEC_REMOVE_FD_NOT_FOUND,
    POLL_VEC_INPUT_FD_IDX_OUT_OF_BOUND,

    POLL_FAILED_TIMED_OUT,

    CMD_INPUT_IS_NULL,
    UDP_INPUT_BUF_IS_NULL,
    UDP_INPUT_VAR_IS_NULL,
    UDP_UNKNOWN_DATA_TYPE,
    UDP_INPUT_IS_NULL,

    SERVER_FAILED_CONNECT_TCP,

    CLIENT_INPUT_IS_NOT_NULL,
    CLIENT_INPUT_IS_NULL,
    CLIENT_INPUT_CONNECT_IS_NULL,
    CLIENT_FAILED_ALLOCATION,
    CLIENT_FAILED_TCP,
    CLIENT_FAILED_POLL_VEC
} err_t;

void debug_msg(const err_t error);
void debug_msg_and_exit(const err_t error);

uint32_t ipow(uint32_t base, uint8_t exp);

#endif /* UTILS_H_ */
