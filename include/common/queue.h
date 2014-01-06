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
  uint16_t size, capacity; /* Current queue size and maximum capacity */
  uint16_t floor, ceil; /* Indexes of first and last elements in queue */
  uint32_t width; /* Size of single element */
};
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *, uint32_t, uint16_t);
void queueDeinit(struct Queue *);
void queuePeek(struct Queue *, void *);
void queuePop(struct Queue *, void *);
void queuePush(struct Queue *, const void *);
/*----------------------------------------------------------------------------*/
static inline uint16_t queueCapacity(const struct Queue *queue)
{
  return queue->capacity;
}
/*----------------------------------------------------------------------------*/
static inline void queueClear(struct Queue *queue)
{
  queue->floor = queue->ceil = queue->size = 0;
}
/*----------------------------------------------------------------------------*/
static inline bool queueEmpty(const struct Queue *queue)
{
  return queue->size == 0;
}
/*----------------------------------------------------------------------------*/
static inline bool queueFull(const struct Queue *queue)
{
  return queue->size == queue->capacity;
}
/*----------------------------------------------------------------------------*/
static inline uint16_t queueSize(const struct Queue *queue)
{
  return queue->size;
}
/*----------------------------------------------------------------------------*/
#endif /* QUEUE_H_ */
