#ifndef POLL_VEC_H_
#define POLL_VEC_H_

#include "./utils.h"

#define REALLOC_FACTOR 2

typedef struct poll_vec_s {
    struct pollfd *pfds;
    nfds_t nfds;
    nfds_t capacity;
} poll_vec_t;

err_t create_poll_vec(poll_vec_t **vec, nfds_t init_nfds);
err_t free_poll_vec(poll_vec_t **vec);
err_t poll_vec_add_fd(poll_vec_t *vec, int fd, short fd_events);

#endif /* POLL_VEC_H_ */