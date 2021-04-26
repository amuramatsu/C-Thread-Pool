/* ********************************
 * Author:       MURAMATSU Atsushi
 * License:	     MIT
 * Description:  Simple Queue for pthread
 ********************************/

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _queue_t* queue_t;

queue_t queue_init(void);
void queue_push(queue_t, void *);
int queue_trypush(queue_t, void *);
void *queue_pop(queue_t);
void *queue_trypop(queue_t);
size_t queue_size(queue_t);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */
