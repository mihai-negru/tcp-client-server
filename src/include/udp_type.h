#ifndef UDP_TYPE_H_
#define UDP_TYPE_H_

#include "./utils.h"

#define MAX_TOPIC_LEN 51
#define MAX_STRING_LEN 1501

typedef enum udp_data_type_s {
    INT         = 0,
    SHORT_REAL  = 1,
    FLOAT       = 2,
    STRING      = 3
} udp_data_type_t;

typedef union udp_data_s {
    int32_t INT;
    double SHORT_REAL;
    double FLOAT;
    char STRING[MAX_STRING_LEN];
} udp_data_t;

typedef struct udp_type_s {
    char topic[MAX_TOPIC_LEN];
    udp_data_type_t type;
    udp_data_t data;
} udp_type_t;

err_t parse_udp_type_from(udp_type_t *udp_type_var, char *buf);
err_t print_udp_type(udp_type_t *udp_type_var);

#endif /* UDP_TYPE_H_ */