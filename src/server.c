#include "./include/server_utils.h"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 2) {
        KILL("[SERVER] Wrong cmdline input.");
    }

    err_t err = OK;

    server_t *server = NULL;
    err = init_server(&server, atoi(argv[1]));

    if (err != OK) {
        debug_msg_and_exit(err);
    }

    loop {
        err = wait_for_ready_fds(server);

        if (err != OK) {
            debug_msg(err);
            break;
        } else {
            if (check_if_exit(server) == 1) {
                break;
            }

            if ((err = process_ready_fds(server)) != OK) {
                debug_msg(err);
                break;
            }
        }
    }

    free_server(&server);

    return EXIT_CODE_GREEN;
}
