/**
 * @file poll_vec.c
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

#include "./include/poll_vec.h"

/**
 * @brief Creates a poll vector and links it to the
 * input poll vector pointer.
 *
 * @param vec pointer to a NULL poll vector.
 * @param init_nfds init capacity for the poll vector.
 * @return err_t OK if poll vector was allocated successfully or error otherwise.
 */
err_t create_poll_vec(poll_vec_t **vec, nfds_t init_nfds) {
    if ((vec == NULL) || (*vec != NULL)) {
        return POLL_VEC_INPUT_IS_NOT_NULL;
    }

    *vec = malloc(sizeof **vec);
    if (*vec == NULL) {
        return POLL_VEC_FAILED_ALLOCATION;
    }

    (*vec)->nfds = 0;
    (*vec)->capacity = init_nfds;
    (*vec)->pfds = malloc(sizeof *(*vec)->pfds * (*vec)->capacity);

    if ((*vec)->pfds == NULL) {
        return POLL_VEC_FAILED_ALLOCATION;
    }

    return OK;
}

/**
 * @brief Frees a poll vector and assigns it to NULL pointer.
 *
 * @param vec pointer to a poll vector.
 * @return err_t OK if poll vector was freed successfully or error otherwise.
 */
err_t free_poll_vec(poll_vec_t **vec) {
    if ((vec == NULL) || (*vec == NULL)) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    for (nfds_t iter = 0; iter < (*vec)->nfds; ++iter) {
        close((*vec)->pfds[iter].fd);
    }

    if ((*vec)->pfds != NULL) {
        free((*vec)->pfds);
    }

    free(*vec);
    *vec = NULL;

    return OK;
}

/**
 * @brief Adds a new valid file descriptor to the poll vector
 * for IO multiplexing.
 *
 * @param vec poll vector structure.
 * @param fd new valid file descriptor.
 * @param fd_events file descriptor events.
 * @return err_t OK if file descriptor was added successfully or error otherwise.
 */
err_t poll_vec_add_fd(poll_vec_t *vec, int fd, short fd_events) {
    if (vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    /* Adds more memory for new fds */
    if (vec->nfds == vec->capacity) {
        struct pollfd *pfds_real = realloc(vec->pfds, sizeof *vec->pfds * vec->capacity * REALLOC_FACTOR);

        if (pfds_real == NULL) {
            return POLL_VEC_FAILED_REALLOC;
        }

        vec->pfds = pfds_real;
        vec->capacity *= REALLOC_FACTOR;
    }

    vec->pfds[vec->nfds].fd = fd;
    vec->pfds[vec->nfds].events = fd_events;
    vec->pfds[vec->nfds].revents = 0;

    (vec->nfds)++;

    return OK;
}

/**
 * @brief Removes a file descriptor from the poll vector at a specified
 * index, the index should be valid or function will return with error.
 *
 * @param vec poll vector structure.
 * @param fd_idx index of the file descriptor.
 * @return err_t OK if file descriptor was removed or error otherwise.
 */
err_t poll_vec_remove_fd(poll_vec_t *vec, nfds_t fd_idx) {
    if (vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    if (fd_idx >= vec->nfds) {
        return POLL_VEC_INPUT_FD_IDX_OUT_OF_BOUND;
    }

    /*
     * Close the specified socket fd and
     * which will generate on the other
     * end a termination for the thread
     */
    close(vec->pfds[fd_idx].fd);

    for (; fd_idx < vec->nfds - 1; ++fd_idx) {
        vec->pfds[fd_idx] = vec->pfds[fd_idx + 1];
    }

    (vec->nfds)--;

    return OK;
}
