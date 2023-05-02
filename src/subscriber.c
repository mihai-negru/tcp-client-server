#include "./include/subscriber_utils.h"

/**
 * @brief Main subscriber function in order to process client requests.
 * 
 * @param argc MUST be 4 one for exec filename, id, ip and a valid port number.
 * @param argv filename, client id, server ip address and server port number.
 * @return int EXIT_CODE_GREEN if success or EXIT_CODE_RED otherwise
 */
int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* Checks upon right number of commands */
    if (argc != 4) {
        KILL("[CLIENT] Wrong cmdline input.");
    }

    err_t err = OK;

    client_t *client = NULL;
    err = init_client(&client, argv[1], argv[2], atoi(argv[3]));

    /* Check if client is up and connected to the server socket */
    if (err != OK) {
        debug_msg_and_exit(err);
    }

    /* Main loop */
    loop {

        /* Waits for a POLLIN revent MUST not timeout */
        err = wait_for_ready_fds(client);

        if (err != OK) {
            /* If timeout something wrong happened so exit the thread */

            debug_msg(err);
            break;
        } else {

            /* Try to fetch a stdin command */
            cmd_line_t cmd = get_cmd_line(client);

            if (cmd == EXIT) {
                /* Exit the main loop and close the connection */

                break;
            } else if (cmd == SUBSCRIBE) {
                /*
                 * Subscribe client to a new topic.
                 * User input is checked via subscribe_cmd function
                 */

                if ((err = process_subscribe_cmd(client)) != OK) {
                    debug_msg(err);
                    break;
                }

                printf("Subscribed to topic.\n");
            } else if (cmd == UNSUBSCRIBE) {
                /*
                 * Unsubscribe client to a new topic.
                 * User input is checked via unsubscribe_cmd function
                 */

                if ((err = process_unsubscribe_cmd(client)) != OK) {
                    debug_msg(err);
                    break;
                }

                printf("Unsubscribed from topic.\n");
            } else if (cmd == NONE) {

                /* The stdin was not POLLIN so process the rest of the fds */
                err = process_ready_fds(client);
                
                /* Connection with server was closed exit the main loop */
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

    /* Free subscriber resources and unbind sockets to server */
    free_client(&client);

    /* All good */
    return EXIT_CODE_GREEN;
}
