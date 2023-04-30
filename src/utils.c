#include "./include/utils.h"

void debug_msg(const err_t error) {
    switch (error) {
        case OK:
            fprintf(stderr, "[DEBUG] No error found.");
            break;
        case SERVER_INPUT_IS_NOT_NULL:
            fprintf(stderr, "[DEBUG] Input server must be NULL to allocate.");
            break;
        case SERVER_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input server must not be NULL.");
            break;
        case INVALID_PORT_NUMBER:
            fprintf(stderr, "[DEBUG] Input port number is not valid.");
            break;
        case SERVER_FAILED_ALLOCATION:
            fprintf(stderr, "[DEBUG] Could not allocate memory for server.");
            break;
        case SERVER_FAILED_UDP:
            fprintf(stderr, "[DEBUG] Could not bind an udp socket for server.");
            break;
        case SERVER_FAILED_TCP:
            fprintf(stderr, "[DEBUG] Could not bind an tcp socket for server.");
            break;
        case SERVER_FAILED_POLL_VEC:
            fprintf(stderr, "[DEBUG] Could not allocate the poll vector.");
            break;
        case POLL_VEC_INPUT_IS_NOT_NULL:
            fprintf(stderr, "[DEBUG] Input poll vecor must be NULL to allocate.");
            break;
        case POLL_VEC_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input poll vectpr must not be NULL.");
            break;
        case POLL_VEC_FAILED_ALLOCATION:
            fprintf(stderr, "[DEBUG] Could not allocate memory for poll vec.");
            break;
        case POLL_VEC_FAILED_REALLOC:
            fprintf(stderr, "[DEBUG] Could not reallocate more fds for poll vec.");
            break;
        case POLL_FAILED_TIMED_OUT:
            fprintf(stderr, "[DEBUG] Poll cannot timeout.");
            break;
        case CMD_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input cmd for stdin must not be NULL.");
            break;
        default:
            fprintf(stderr, "[DEBUG] Unknown command.");
    }

    fprintf(stderr, "\n");
}

void debug_msg_and_exit(const err_t error) {
    debug_msg(error);
    exit(EXIT_CODE_GREEN);
}

uint32_t ipow(uint32_t base, uint8_t exp) {
    int base_to_exp = 1;

    while (exp != 0) {
        if ((exp & 1) != 0) {
            base_to_exp *= base;
        }

        exp >>= 1;

        base *= base;
    }

    return base_to_exp;
}