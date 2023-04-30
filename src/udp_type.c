#include "./include/udp_type.h"

static void parse_udp_int_type(udp_type_t *udp_type_var, char *buf) {
    uint8_t sign = *(uint8_t *)buf;

    udp_type_var->data.INT = ntohl(*(uint32_t *)(buf + 1));

    if (sign != 0) {
        udp_type_var->data.INT *= -1;
    }
}

static void parse_udp_short_real_type(udp_type_t *udp_type_var, char *buf) {
    udp_type_var->data.SHORT_REAL = (1.0 * ntohs(*(uint16_t *)buf)) / 100;
}

static void parse_udp_float_type(udp_type_t *udp_type_var, char *buf) {
    uint8_t sign = *(uint8_t *)buf;

    udp_type_var->data.FLOAT = (1.0 * ntohl(*(uint32_t *)(buf + 1))) / ipow(10, *(uint8_t *)(buf + 1 + sizeof (uint32_t)));

    if (sign != 0) {
        udp_type_var->data.FLOAT *= -1.0;
    }
}

static void parse_udp_string_type(udp_type_t *udp_type_var, char *buf) {
    memset(udp_type_var->data.STRING, '\0', MAX_STRING_LEN);
    memcpy(udp_type_var->data.STRING, buf, MAX_STRING_LEN);
}

err_t parse_udp_type_from(udp_type_t *udp_type_var, char *buf) {
    if (udp_type_var == NULL) {
        return UDP_INPUT_VAR_IS_NULL;
    }
    
    if (buf == NULL) {
        return UDP_INPUT_BUF_IS_NULL;
    }

    memset(udp_type_var->topic, '\0', MAX_TOPIC_LEN);
    memcpy(udp_type_var->topic, buf, MAX_TOPIC_LEN);

    udp_type_var->type = (udp_data_type_t)(*(uint8_t *)(buf + MAX_TOPIC_LEN - 1));

    size_t buf_offset = MAX_TOPIC_LEN;
    switch (udp_type_var->type) {
        case INT:
            parse_udp_int_type(udp_type_var, buf + buf_offset);
            break;
        case SHORT_REAL:
            parse_udp_short_real_type(udp_type_var, buf + buf_offset);
            break;
        case FLOAT:
            parse_udp_float_type(udp_type_var, buf + buf_offset);
            break;
        case STRING:
            parse_udp_string_type(udp_type_var, buf + buf_offset);
            break;
        default:
            return UDP_UNKNOWN_DATA_TYPE;
    }

    return OK;
}

err_t print_udp_type(udp_type_t *udp_type_var) {
    if (udp_type_var == NULL) {
        return UDP_INPUT_IS_NULL;
    }

    fprintf(stderr, "UDP Package:\n");
    fprintf(stderr, "  topic: %s,\n", udp_type_var->topic);
    
    switch (udp_type_var->type) {
        case INT:
            fprintf(stderr, "  type: INT,\n");
            fprintf(stderr, "  data: %d.\n\n", udp_type_var->data.INT);
            break;
        case SHORT_REAL:
            fprintf(stderr, "  type: SHORT_REAL,\n");
            fprintf(stderr, "  data: %.2f.\n\n", udp_type_var->data.SHORT_REAL);
            break;
        case FLOAT:
            fprintf(stderr, "  type: FLOAT,\n");
            fprintf(stderr, "  data: %f.\n\n", udp_type_var->data.FLOAT);
            break;
        case STRING:
            fprintf(stderr, "  type: STRING,\n");
            fprintf(stderr, "  data: %s.\n\n", udp_type_var->data.STRING);
            break;
        default:
            return UDP_UNKNOWN_DATA_TYPE;
    }

    return OK;
}