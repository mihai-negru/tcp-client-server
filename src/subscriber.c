#include "./include/subscriber_utils.h"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc != 4) {
        KILL("[CLIENT] Wrong cmdline input.");
    }

    err_t err = OK;

    client_t *client = NULL;
    err = init_client(&client, argv[1], argv[2], atoi(argv[3]));

    if (err != OK) {
        debug_msg_and_exit(err);
    }

    loop {
        err = wait_for_ready_fds(client);

        if (err != OK) {
            debug_msg(err);
            break;
        } else {
            cmd_line_t cmd = get_cmd_line(client);

            if (cmd == EXIT) {
                break;
            } else if (cmd == SUBSCRIBE) {
                if ((err = process_subscribe_cmd(client)) != OK) {
                    debug_msg(err);
                    break;
                }

                printf("Subscribed to %s.\n", client->cmd);
            } else if (cmd == UNSUBSCRIBE) {
                if ((err = process_unsubscribe_cmd(client)) != OK) {
                    debug_msg(err);
                    break;
                }

                printf("Unsubscribed from %s.\n", client->cmd);
            } else if (cmd == NONE) {
                err = process_ready_fds(client);
                
                if (err == OK_WITH_EXIT) {
                    break;
                }

                if (err != OK) {
                    debug_msg(err);
                    break;
                }
            } else {
                DEBUG("[CLIENT] Invalid command type.");
            }
        }
    }

    free_client(&client);

    return EXIT_CODE_GREEN;
}
