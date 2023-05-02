#include "./include/server_utils.h"

/**
 * @brief Main server function in order to process clients requests.
 * 
 * @param argc MUST be 2 one for exec filename and one for a valid port number.
 * @param argv port number represented as a string
 * @return int EXIT_CODE_GREEN if success or EXIT_CODE_RED otherwise
 */
int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    /* Checks upon right number of commands */
    if (argc != 2) {
        KILL("[SERVER] Wrong cmdline input.");
    }

    err_t err = OK;

    server_t *server = NULL;
    err = init_server(&server, atoi(argv[1]));

    /* Check if server is up and connected to udp and tcp sockets */
    if (err != OK) {
        debug_msg_and_exit(err);
    }

    /* Main loop */
    loop {

        /* Waits for a POLLIN revent MUST not timeout */
        err = wait_for_ready_fds(server);

        if (err != OK) {
            /* If timeout something wrong happened so exit the thread */

            debug_msg(err);
            break;
        } else {

            /* Checks if server got an exit input request */
            if (check_if_exit(server) == 1) {
                break;
            }

            /* Process all the revents from the poll vector structure */
            if ((err = process_ready_fds(server)) != OK) {
                debug_msg(err);
                break;
            }
        }
    }

    /* Free server resources and unbind udp and tcp sockets */
    free_server(&server);

    /* All good */
    return EXIT_CODE_GREEN;
}
