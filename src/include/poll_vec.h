/**
 * @file poll_vec.h
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

#ifndef POLL_VEC_H_
#define POLL_VEC_H_

#include "./utils.h"

/**
 * @brief Struct class defining a vector
 * of pollfds, in order to mantain a dynamic
 * behaviour of IO multiplexing.
 *
 */
typedef struct poll_vec_s {
    struct pollfd   *pfds;
    nfds_t          nfds;
    nfds_t          capacity;
} poll_vec_t;

/**
 * @brief Creates a poll vector and links it to the
 * input poll vector pointer.
 *
 * @param vec pointer to a NULL poll vector.
 * @param init_nfds init capacity for the poll vector.
 * @return err_t OK if poll vector was allocated successfully or error otherwise.
 */
err_t create_poll_vec(poll_vec_t **vec, nfds_t init_nfds);

/**
 * @brief Frees a poll vector and assigns it to NULL pointer.
 *
 * @param vec pointer to a poll vector.
 * @return err_t OK if poll vector was freed successfully or error otherwise.
 */
err_t free_poll_vec(poll_vec_t **vec);

/**
 * @brief Adds a new valid file descriptor to the poll vector
 * for IO multiplexing.
 *
 * @param vec poll vector structure.
 * @param fd new valid file descriptor.
 * @param fd_events file descriptor events.
 * @return err_t OK if file descriptor was added successfully or error otherwise.
 */
err_t poll_vec_add_fd(poll_vec_t *vec, int fd, short fd_events);

/**
 * @brief Removes a file descriptor from the poll vector at a specified
 * index, the index should be valid or function will return with error.
 *
 * @param vec poll vector structure.
 * @param fd_idx index of the file descriptor.
 * @return err_t OK if file descriptor was removed or error otherwise.
 */
err_t poll_vec_remove_fd(poll_vec_t *vec, nfds_t fd_idx);

#endif /* POLL_VEC_H_ */
