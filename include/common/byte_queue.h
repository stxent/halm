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
  uint16_t size, capacity; /* Current queue size and maximum capacity */
  uint16_t floor, ceil; /* Indexes of first and last elements in queue */
};
/*----------------------------------------------------------------------------*/
enum result byteQueueInit(struct ByteQueue *, uint16_t);
void byteQueueDeinit(struct ByteQueue *);
uint16_t byteQueuePushArray(struct ByteQueue *, const uint8_t *, uint16_t);
uint16_t byteQueuePopArray(struct ByteQueue *, uint8_t *, uint16_t);
/*----------------------------------------------------------------------------*/
static inline uint16_t byteQueueCapacity(const struct ByteQueue *queue)
{
  return queue->capacity;
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
static inline uint8_t byteQueuePeek(struct ByteQueue *queue)
{
  assert(queue->size);

  return queue->data[queue->floor];
}
/*----------------------------------------------------------------------------*/
static inline uint8_t byteQueuePop(struct ByteQueue *queue)
{
  assert(queue->size);

  uint8_t tmp = queue->data[queue->floor++];
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
static inline uint16_t byteQueueSize(const struct ByteQueue *queue)
{
  return queue->size;
}
/*----------------------------------------------------------------------------*/
#endif /* BYTE_QUEUE_H_ */
