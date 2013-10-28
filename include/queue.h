/*
 * queue.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef QUEUE_H_
#define QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
struct Queue
{
  uint8_t *data;
  uint16_t size, capacity; /* Current queue size and maximal capacity */
  uint16_t floor, ceil; /* Indexes of first and last elements in queue */
};
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
static inline uint16_t queueCapacity(const struct Queue *q)
{
  return q->capacity;
}
/*----------------------------------------------------------------------------*/
static inline bool queueEmpty(const struct Queue *q)
{
  return q->size == 0;
}
/*----------------------------------------------------------------------------*/
static inline bool queueFull(const struct Queue *q)
{
  return q->size == q->capacity;
}
/*----------------------------------------------------------------------------*/
static inline uint16_t queueSize(const struct Queue *q)
{
  return q->size;
}
/*----------------------------------------------------------------------------*/
#endif /* QUEUE_H_ */
