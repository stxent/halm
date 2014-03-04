/*
 * queue.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef QUEUE_H_
#define QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
struct Queue
{
  void *data;

  /** Maximum capacity of the queue. */
  unsigned short capacity;
  /** Current number of elements in the queue. */
  unsigned short size;
  /** Index of the last element. */
  unsigned short ceil;
  /** Index of the first element. */
  unsigned short floor;
  /** Size of each element in bytes. */
  unsigned int width;
};
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *, unsigned int, unsigned int);
void queueDeinit(struct Queue *);
void queuePeek(const struct Queue *, void *);
void queuePop(struct Queue *, void *);
void queuePush(struct Queue *, const void *);
/*----------------------------------------------------------------------------*/
static inline unsigned int queueCapacity(const struct Queue *queue)
{
  return (unsigned int)queue->capacity;
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
static inline unsigned int queueSize(const struct Queue *queue)
{
  return (unsigned int)queue->size;
}
/*----------------------------------------------------------------------------*/
#endif /* QUEUE_H_ */
