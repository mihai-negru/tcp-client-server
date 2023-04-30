#include "./include/poll_vec.h"

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

err_t free_poll_vec(poll_vec_t **vec) {
    if ((vec == NULL) || (*vec == NULL)) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    if ((*vec)->pfds != NULL) {
        free((*vec)->pfds);
    }

    free(*vec);
    *vec = NULL;

    return OK;
}

err_t poll_vec_add_fd(poll_vec_t *vec, int fd, short fd_events) {
    if (vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

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

    (vec->nfds)++;

    return OK;
}

err_t poll_vec_remove_fd(poll_vec_t *vec, int fd) {
    if (vec == NULL) {
        return POLL_VEC_INPUT_IS_NULL;
    }

    nfds_t remove_fd_idx = vec->nfds;

    for (nfds_t iter = 0; iter < vec->nfds; ++iter) {
        if (vec->pfds[iter].fd == fd) {
            remove_fd_idx = iter;
        }
    }

    if (remove_fd_idx >= vec->nfds) {
        return POLL_VEC_REMOVE_FD_NOT_FOUND;
    }

    close(fd);

    if (remove_fd_idx != vec->nfds - 1) {
        memmove(vec + remove_fd_idx, vec + remove_fd_idx + 1, sizeof *vec->pfds * (vec->nfds - remove_fd_idx - 1));
    }

    (vec->nfds)--;

    return OK;
}