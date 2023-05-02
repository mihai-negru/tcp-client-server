#ifndef UDP_TYPE_H_
#define UDP_TYPE_H_

#include "./utils.h"

#define MAX_TOPIC_LEN 51
#define MAX_STRING_LEN 1501

/**
 * @brief Enum class type to handle
 * the udp message types.
 * 
 */
typedef enum udp_data_type_s {
    INT         = 0,
    SHORT_REAL  = 1,
    FLOAT       = 2,
    STRING      = 3
} udp_data_type_t;

/**
 * @brief Union class type to store
 * a udp message data depending on the
 * udp message type.
 * 
 */
typedef union udp_data_s {
    int32_t     INT;
    double      SHORT_REAL;
    double      FLOAT;
    char        STRING[MAX_STRING_LEN];
} udp_data_t;

/**
 * @brief Struct class type containing
 * udp message metadata and exact data.
 * 
 */
typedef struct udp_type_s {
    char                topic[MAX_TOPIC_LEN];
    udp_data_type_t     type;
    udp_data_t          data;
} udp_type_t;

/**
 * @brief Parses a buffer into a udp message type.
 * 
 * @param udp_type_var pointer to memory location to parse the message.
 * @param buf pointer to buffer containing the message bytes.
 * @return err_t OK if parser executed successfully or UDP_* errors otherwise.
 */
err_t parse_udp_type_from(udp_type_t *udp_type_var, char *buf);

/**
 * @brief Prints a parsed udp message type on stderr
 * for debugging purposes.
 * 
 * @param udp_type_var pointer to udp message type.
 * @return err_t OK if printing went successfully.
 */
err_t print_udp_type(udp_type_t *udp_type_var);

#endif /* UDP_TYPE_H_ */
