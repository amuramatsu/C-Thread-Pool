/* ********************************
 * Author:       MURAMATSU Atsushi
 * License:	     MIT
 * Description:  Simple Queue for pthread
 ********************************/

#define _POSIX_C_SOURCE 200809L
#include "queue.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

struct _node_t {
    struct _node_t *next;
    void *data;
};

struct _queue_t {
    struct _node_t *root;
    struct _node_t **tail_ptr;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    size_t num;
};

static struct _node_t *_queue_make_node(void *);
static void _queue_push_impl(queue_t, struct _node_t *);
static struct _node_t *_queue_pop_impl(queue_t);

queue_t
queue_init(void)
{
    struct _queue_t *q = (struct _queue_t *)malloc(sizeof(struct _queue_t));
    if (q == NULL)
	return NULL;
    memset(q, 0, sizeof(struct _queue_t));
    q->tail_ptr = &q->root;
    if (pthread_mutex_init(&q->mutex, NULL) != 0)
	goto error;
    if (pthread_cond_init(&q->cond, NULL) != 0)
	goto error;
    return q;
 error:
    free(q);
    return NULL;
}

static struct _node_t *
_queue_make_node(void *data)
{
    struct _node_t *n = (struct _node_t *)malloc(sizeof(struct _node_t));
    if (n == NULL)
	abort();
    n->data = data;
    n->next = NULL;
    return n;
}

static void
_queue_push_impl(queue_t q, struct _node_t *n)
{
    *q->tail_ptr = n;
    q->tail_ptr = &n->next;
    if (q->num == 0) {
	if (pthread_cond_signal(&q->cond) != 0)
	    abort();
    }
    q->num++;
}

static struct _node_t *
_queue_pop_impl(queue_t q)
{
    struct _node_t *p = q->root;
    q->root = p->next;
    if (q->root == NULL)
	q->tail_ptr = &q->root;
    q->num--;
    return p;
}   

void
queue_push(queue_t q, void *data)
{
    struct _node_t *n = _queue_make_node(data);
    if (pthread_mutex_lock(&q->mutex) != 0)
	abort();
    _queue_push_impl(q, n);
    if (pthread_mutex_unlock(&q->mutex) != 0)
	abort();
}

int
queue_trypush(queue_t q, void *data)
{
    struct _node_t *n;
    switch (pthread_mutex_trylock(&q->mutex)) {
    case EBUSY:
	return -1;
    case 0:
	break;
    default:
	abort();
    }
    n = _queue_make_node(data);
    _queue_push_impl(q, n);
    if (pthread_mutex_unlock(&q->mutex) != 0)
	abort();
    return 0;
}

void *
queue_pop(queue_t q) {
    struct _node_t *p;
    if (pthread_mutex_lock(&q->mutex) != 0)
	abort();
    while (q->num == 0) {
	if (pthread_cond_wait(&q->cond, &q->mutex) != 0)
	    abort();
    }
    p = _queue_pop_impl(q);
    if (pthread_mutex_unlock(&q->mutex) != 0)
	abort();
    void *data = p->data;
    free(p);
    return data;
}

void *
queue_trypop(queue_t q)
{
    void *result;
    struct _node_t *p;
    switch (pthread_mutex_trylock(&q->mutex)) {
    case EBUSY:
	return NULL;
    case 0:
	break;
    default:
	abort();
    }
    while (q->num == 0) {
	if (pthread_mutex_unlock(&q->mutex) != 0)
	    abort();
	return NULL;
    }
    p = _queue_pop_impl(q);
    if (pthread_mutex_unlock(&q->mutex) != 0)
	abort();
    result = p->data;
    free(p);
    return result;
}

size_t
queue_size(queue_t q)
{
    size_t num;
    if (pthread_mutex_lock(&q->mutex) != 0)
	abort();
    num = q->num;
    if (pthread_mutex_unlock(&q->mutex) != 0)
	abort();
    return num;
}
