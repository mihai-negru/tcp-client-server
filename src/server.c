#include "./include/server_utils.h"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 2) {
        KILL("[SERVER] Expected <1> argument.");
    }

    err_t err = OK;

    server_t *server = NULL;
    err = init_server(&server, atoi(argv[1]));

    if (err != OK) {
        debug_msg_and_exit(err);
    }

    char *cmd = malloc(sizeof *cmd * MAX_CMD_LEN);
    if (cmd == NULL) {
        debug_msg_and_exit(free_server(&server));
    }

    int ready_fds = 0;
    for (;;) {
        ready_fds = poll(server->poll_vec->pfds, server->poll_vec->nfds, -1);

        if (ready_fds <= 0) {
            KILL("[SERVER] Poll timed out, could not happen.");
        } else {
            if ((server->poll_vec->pfds[0].revents & POLLIN) != 0) {
                if (fgets(cmd, MAX_CMD_LEN, stdin) != NULL) {
                    if (strncmp(cmd, EXIT_CMD, strlen(EXIT_CMD)) == 0) {
                        break;
                    }
                }
            }

            for (nfds_t i = 1; i < server->poll_vec->nfds; ++i) {
                if ((server->poll_vec->pfds[i].revents & POLLIN) != 0) {
                    fprintf(stderr, "[SERVER] Got poll file <%ld>\n", i);
                }
            }
        }
    }

    free(cmd);
    free_server(&server);

    return 0;
}
