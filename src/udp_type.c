/**
 * @file udp_type.c
 * @author Mihai Negru (determinant289@gmail.com)
 * @version 1.0.0
 * @date 2023-05-02
 *
 * @copyright Copyright (C) 2023-2024 Mihai Negru <determinant289@gmail.com>
 * This file is part of tcp-client-server.
 *
 * tcp-client-server is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tcp-client-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tcp-client-server.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "./include/udp_type.h"

/**
 * @brief Parses a int udp message type.
 *
 * @param udp_type_var pointer to memory location to parse the message.
 * @param buf pointer to buffer containing the message bytes.
 */
static void parse_udp_int_type(udp_type_t *udp_type_var, char *buf) {
    uint8_t sign = *(uint8_t *)buf;

    udp_type_var->data.INT = ntohl(*(uint32_t *)(buf + 1));

    if (sign != 0) {
        udp_type_var->data.INT *= -1;
    }
}

/**
 * @brief Parses a short real udp message type.
 *
 * @param udp_type_var pointer to memory location to parse the message.
 * @param buf pointer to buffer containing the message bytes.
 */
static void parse_udp_short_real_type(udp_type_t *udp_type_var, char *buf) {
    udp_type_var->data.SHORT_REAL = (1.0 * ntohs(*(uint16_t *)buf)) / 100;
}

/**
 * @brief Parses a float udp message type.
 *
 * @param udp_type_var pointer to memory location to parse the message.
 * @param buf pointer to buffer containing the message bytes.
 */
static void parse_udp_float_type(udp_type_t *udp_type_var, char *buf) {
    uint8_t sign = *(uint8_t *)buf;

    udp_type_var->data.FLOAT = (1.0 * ntohl(*(uint32_t *)(buf + 1))) / ipow(10, *(uint8_t *)(buf + 1 + sizeof (uint32_t)));

    if (sign != 0) {
        udp_type_var->data.FLOAT *= -1.0;
    }
}

/**
 * @brief Parses a string udp message type.
 *
 * @param udp_type_var pointer to memory location to parse the message.
 * @param buf pointer to buffer containing the message bytes.
 */
static void parse_udp_string_type(udp_type_t *udp_type_var, char *buf) {
    memset(udp_type_var->data.STRING, '\0', MAX_STRING_LEN);
    memcpy(udp_type_var->data.STRING, buf, MAX_STRING_LEN);
}

/**
 * @brief Parses a buffer into a udp message type.
 *
 * @param udp_type_var pointer to memory location to parse the message.
 * @param buf pointer to buffer containing the message bytes.
 * @return err_t OK if parser executed successfully or UDP_* errors otherwise.
 */
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

/**
 * @brief Prints a parsed udp message type on stderr
 * for debugging purposes.
 *
 * @param udp_type_var pointer to udp message type.
 * @return err_t OK if printing went successfully.
 */
err_t print_udp_type(udp_type_t *udp_type_var) {
    if (udp_type_var == NULL) {
        return UDP_INPUT_VAR_IS_NULL;
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
