/*
 * queue.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef QUEUE_H_
#define QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "error.h"
/*----------------------------------------------------------------------------*/
struct Queue
{
  uint8_t *data;
  uint16_t size, capacity; /* Current queue size and maximal capacity */
  uint16_t floor, ceil; /* Indexes of first and last elements in queue */
};
/*----------------------------------------------------------------------------*/
#define queueEmpty(q)           (!(q)->size)
#define queueFull(q)            ((q)->size == (q)->capacity)
#define queueSize(q)            ((q)->size)
#define queueCapacity(q)        ((q)->capacity)
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *, uint16_t);
void queueDeinit(struct Queue *);
void queueClear(struct Queue *);
uint8_t queuePeek(struct Queue *);
uint8_t queuePop(struct Queue *);
void queuePush(struct Queue *, uint8_t);
uint16_t queuePushArray(struct Queue *, const uint8_t *, uint16_t);
uint16_t queuePopArray(struct Queue *, uint8_t *, uint16_t);
/*----------------------------------------------------------------------------*/
#endif /* QUEUE_H_ */
