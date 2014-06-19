/*
 * byte_queue.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef BYTE_QUEUE_H_
#define BYTE_QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
struct ByteQueue
{
  uint8_t *data;
  /** Maximum capacity of the queue. */
  unsigned short capacity;
  /** Current number of elements in the queue. */
  unsigned short size;
  /** Index of the last element. */
  unsigned short ceil;
  /** Index of the first element. */
  unsigned short floor;
};
/*----------------------------------------------------------------------------*/
enum result byteQueueInit(struct ByteQueue *, unsigned int);
void byteQueueDeinit(struct ByteQueue *);
unsigned int byteQueuePopArray(struct ByteQueue *, uint8_t *, unsigned int);
unsigned int byteQueuePushArray(struct ByteQueue *, const uint8_t *,
    unsigned int);
/*----------------------------------------------------------------------------*/
static inline unsigned int byteQueueCapacity(const struct ByteQueue *queue)
{
  return (unsigned int)queue->capacity;
}
/*----------------------------------------------------------------------------*/
static inline void byteQueueClear(struct ByteQueue *queue)
{
  queue->floor = queue->ceil = queue->size = 0;
}
/*----------------------------------------------------------------------------*/
static inline bool byteQueueEmpty(const struct ByteQueue *queue)
{
  return queue->size == 0;
}
/*----------------------------------------------------------------------------*/
static inline bool byteQueueFull(const struct ByteQueue *queue)
{
  return queue->size == queue->capacity;
}
/*----------------------------------------------------------------------------*/
static inline uint8_t byteQueuePeek(const struct ByteQueue *queue)
{
  assert(queue->size);

  return queue->data[queue->floor];
}
/*----------------------------------------------------------------------------*/
static inline uint8_t byteQueuePop(struct ByteQueue *queue)
{
  assert(queue->size);

  const uint8_t tmp = queue->data[queue->floor++];

  if (queue->floor == queue->capacity)
    queue->floor = 0;

  --queue->size;

  return tmp;
}
/*----------------------------------------------------------------------------*/
static inline void byteQueuePush(struct ByteQueue *queue, uint8_t value)
{
  assert(queue->size < queue->capacity);

  queue->data[queue->ceil++] = value;

  if (queue->ceil == queue->capacity)
    queue->ceil = 0;

  ++queue->size;
}
/*----------------------------------------------------------------------------*/
static inline unsigned int byteQueueSize(const struct ByteQueue *queue)
{
  return (unsigned int)queue->size;
}
/*----------------------------------------------------------------------------*/
#endif /* BYTE_QUEUE_H_ */
