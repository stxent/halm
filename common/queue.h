/*
 * queue.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef QUEUE_H_
#define QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
// #include <stdbool.h>
/*----------------------------------------------------------------------------*/
#include "entity.h"
/*----------------------------------------------------------------------------*/
struct Queue
{
  uint16_t size; /* Current queue size */
  uint16_t capacity; /* Queue capacity */
  uint16_t floor, ceil; /* Indexes of first and last elements in queue */
  uint8_t *data;
};
/*----------------------------------------------------------------------------*/
#define queueSize(q)            ((q)->size)
#define queueFull(q)            ((q)->size == (q)->capacity)
#define queueEmpty(q)           (!(q)->size)
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *, uint16_t);
void queueDeinit(struct Queue *);
void queuePush(struct Queue *, uint8_t);
uint8_t queuePop(struct Queue *);
uint8_t queuePeek(struct Queue *);
/* uint16_t queueSize(struct Queue *); */
/* bool queueFull(struct Queue *); */
/* bool queueEmpty(struct Queue *); */
/*----------------------------------------------------------------------------*/
#endif /* QUEUE_H_ */
