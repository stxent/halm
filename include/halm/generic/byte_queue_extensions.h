/*
 * halm/generic/byte_queue_extensions.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_BYTE_QUEUE_EXTENSIONS_H_
#define HALM_GENERIC_BYTE_QUEUE_EXTENSIONS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/byte_queue.h>
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

static inline void byteQueueAbandon(struct ByteQueue *queue, size_t size)
{
  assert(queue->size >= size);

  queue->head += size;
  if (queue->head >= queue->capacity)
    queue->head -= queue->capacity;
  queue->size -= size;
}

static inline void byteQueueAdvance(struct ByteQueue *queue, size_t size)
{
  assert(queue->size + size <= queue->capacity);

  queue->tail += size;
  if (queue->tail >= queue->capacity)
    queue->tail -= queue->capacity;
  queue->size += size;
}

static inline void byteQueueDeferredPop(struct ByteQueue *queue,
    const uint8_t **buffer, size_t *size, size_t offset)
{
  assert(queue->size - offset > 0);

  size_t head = queue->head + offset;
  if (head >= queue->capacity)
    head -= queue->capacity;

  *buffer = &queue->data[head];

  if (head < queue->tail)
    *size = queue->size;
  else
    *size = queue->capacity - head;
}

static inline void byteQueueDeferredPush(struct ByteQueue *queue,
    uint8_t **buffer, size_t *size, size_t offset)
{
  assert(queue->size + offset < queue->capacity);

  size_t tail = queue->tail + offset;
  if (tail >= queue->capacity)
    tail -= queue->capacity;

  *buffer = &queue->data[tail];

  if (queue->head > tail)
    *size = queue->capacity - (queue->size + offset);
  else
    *size = queue->capacity - tail;
}

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_BYTE_QUEUE_EXTENSIONS_H_ */
