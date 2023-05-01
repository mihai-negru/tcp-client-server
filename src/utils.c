#include "./include/utils.h"

void debug_msg(const err_t error) {
    switch (error) {
        case OK:
            fprintf(stderr, "[DEBUG] No error found.");
            break;
        case OK_WITH_EXIT:
            fprintf(stderr, "[DEBUG] Client is disconected or cannot read data.");
            break;
        case INVALID_PORT_NUMBER:
            fprintf(stderr, "[DEBUG] Input port number is not valid.");
            break;
        case POLL_FAILED_TIMED_OUT:
            fprintf(stderr, "[DEBUG] Poll cannot timeout.");
            break;
        case CMD_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input cmd for stdin must not be NULL.");
            break;
        case SERVER_INPUT_IS_NOT_NULL:
            fprintf(stderr, "[DEBUG] Input server must be NULL to allocate.");
            break;
        case SERVER_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input server must not be NULL.");
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
        case SERVER_COULD_NOT_CONNECT_NEW_CLIENT:
            fprintf(stderr, "[DEBUG] Could not get the id of the client.");
            break;
        case SERVER_FAILED_ACCEPT_TCP:
            fprintf(stderr, "[DEBUG] Could not accept any new clients.");
            break;
        case SERVER_FAILED_CLIENTS_VEC:
            fprintf(stderr, "[DEBUG] Could not allocate the clients vector.");
            break;
        case SERVER_UNKNOWN_COMMAND:
            fprintf(stderr, "[DEBUG] Could not match the client command.");
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
        case POLL_VEC_INPUT_FD_IDX_OUT_OF_BOUND:
            fprintf(stderr, "[DEBUG] The fd index is to large for the poll vector.");
            break;
        case UDP_INPUT_BUF_IS_NULL:
            fprintf(stderr, "[DEBUG] Input udp buffer must not be NULL.");
            break;
        case UDP_INPUT_VAR_IS_NULL:
            fprintf(stderr, "[DEBUG] Input prasing udp variable must not be NULL.");
            break;
        case UDP_UNKNOWN_DATA_TYPE:
            fprintf(stderr, "[DEBUG] UDP message type is unknown.");
            break;
        case CLIENT_INPUT_IS_NOT_NULL:
            fprintf(stderr, "[DEBUG] Input clients vector must be NULL to allocate.");
            break;
        case CLIENT_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input clients vector must not be NULL.");
            break;
        case CLIENT_INPUT_CONNECT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input subscriber informations must not be NULL.");
            break;
        case CLIENT_FAILED_ALLOCATION:
            fprintf(stderr, "[DEBUG] Could not allocate mameory for the subscriber.");
            break;
        case CLIENT_FAILED_TCP:
            fprintf(stderr, "[DEBUG] Could not connect subscriber to the server.");
            break;
        case CLIENT_FAILED_POLL_VEC:
            fprintf(stderr, "[DEBUG] Could not allocate the poll vector for subscriber.");
            break;
        case TCP_INPUT_SOCKET_INVALID:
            fprintf(stderr, "[DEBUG] Input TCP socket is negative.");
            break;
        case TCP_INPUT_BUF_IS_NULL:
            fprintf(stderr, "[DEBUG] Input buffer for send/recv must not be NULL.");
            break;
        case TCP_INPUT_BUF_LEN_IS_ZERO:
            fprintf(stderr, "[DEBUG] Input buffer size for send/recv must not be 0.");
            break;
        case TCP_FAILED_SEND_RECV:
            fprintf(stderr, "[DEBUG] TCP send/recv 0 bytes of data.");
            break;
        case CLIENTS_VEC_INPUT_IS_NOT_NULL:
            fprintf(stderr, "[DEBUG] Input clients vector must be NULL to allocate.");
            break;
        case CLIENTS_VEC_FAILED_ALLOCATION:
            fprintf(stderr, "[DEBUG] Could not allocate memory for clients vector.");
            break;
        case CLIENTS_VEC_INPUT_IS_NULL:
            fprintf(stderr, "[DEBUG] Input clients vector must not be NULL.");
            break;
        case CLIENTS_VEC_FAILED_REALLOC:
            fprintf(stderr, "[DEBUG] Could not realloc more memory for clients vector.");
            break;
        case CLIENTS_VEC_FAILED_REGISTER_ALLOCATION:
            fprintf(stderr, "[DEBUG] Could not allocate memory for a new client in server.");
            break;
        case CLIENT_VEC_CLIENT_ALREADY_CONNECTED:
            fprintf(stderr, "[DEBUG] The client is already active.");
            break;
        case CLIENTS_VEC_INPUT_FD_IDX_INVALID:
            fprintf(stderr, "[DEBUG] Client fd must not be negative.");
            break;
        case CLIENTS_VEC_CLIENT_ALREADY_DEAD:
            fprintf(stderr, "[DEBUG] Client connection is already closed.");
            break;
        case CLIENTS_VEC_CLOSE_CLIENT_NOT_FOUND:
            fprintf(stderr, "[DEBUG] Input client fd cannot be found in clients vector for closing.");
            break;
        case CLIENTS_VEC_CLIENT_NOT_FOUND:
            fprintf(stderr, "[DEBUG] Client cound not be found in the clients vector.");
            break;
        case CLIENTS_VEC_COUND_NOT_ADD_A_TOPIC:
            fprintf(stderr, "[DEBUG] Cound not add a new subscribed topic for client.");
            break;
        case CLIENTS_VEC_COUND_NOT_FIND_TOPIC:
            fprintf(stderr, "[DEBUG] Cound not find the topic in order to remove.");
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