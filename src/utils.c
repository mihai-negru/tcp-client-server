#include "./include/utils.h"

void debug_err_msg(const err_t error) {
    (void)error;
}

void debug_msg_and_exit(const err_t error) {
    fprintf(stderr, "NOT GOOD : %d\n", error);
    exit(EXIT_CODE_GREEN);
}