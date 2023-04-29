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
        exit(-2);                       \
    } while (0)

#define MAX_CMD_LEN 10
#define EXIT_CODE_GREEN 0
#define EXIT_CODE_YELLOW -1
#define EXIT_CODE_RED -2

typedef enum err_s {
    OK,

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
    POLL_VEC_FAILED_REALLOC
} err_t;

void debug_msg(const err_t error);
void debug_msg_and_exit(const err_t error);
uint8_t check_if_exit(char *cmd);

#endif /* UTILS_H_ */
