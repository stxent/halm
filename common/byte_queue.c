/*
 * byte_queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <string.h>
#include <byte_queue.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_CAPACITY 16
/*----------------------------------------------------------------------------*/
enum result byteQueueInit(struct ByteQueue *queue, uint16_t capacity)
{
  if (!capacity)
    capacity = DEFAULT_CAPACITY;
  queue->data = malloc(capacity);
  if (!queue->data)
    return E_MEMORY;

  queue->capacity = capacity;
  queue->floor = queue->ceil = queue->size = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void byteQueueDeinit(struct ByteQueue *queue)
{
  free(queue->data);
}
/*----------------------------------------------------------------------------*/
uint16_t byteQueuePopArray(struct ByteQueue *queue, uint8_t *buffer,
    uint16_t length)
{
  uint16_t count, moved = 0;

  if (queue->ceil < queue->floor)
  {
    count = queue->capacity - queue->floor;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(buffer, queue->data + queue->floor, count);
      buffer += count;
      queue->floor = queue->floor + count;
      if (queue->floor == queue->capacity)
        queue->floor = 0;
      queue->size -= count;
      length -= count;
      moved += count;
    }
  }
  if (queue->ceil >= queue->floor)
  {
    count = queue->ceil - queue->floor;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(buffer, queue->data + queue->floor, count);
      queue->floor += count;
      queue->size -= count;
      moved += count;
    }
  }
  return moved;
}
/*----------------------------------------------------------------------------*/
uint16_t byteQueuePushArray(struct ByteQueue *queue, const uint8_t *buffer,
    uint16_t length)
{
  uint16_t count, moved = 0;

  if (queue->ceil >= queue->floor)
  {
    count = queue->capacity - queue->ceil;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(queue->data + queue->ceil, buffer, count);
      buffer += count;
      queue->ceil = queue->ceil + count;
      if (queue->ceil == queue->capacity)
        queue->ceil = 0;
      queue->size += count;
      length -= count;
      moved += count;
    }
  }
  if (queue->ceil < queue->floor)
  {
    count = queue->floor - queue->ceil;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(queue->data + queue->ceil, buffer, count);
      queue->ceil += count;
      queue->size += count;
      moved += count;
    }
  }
  return moved;
}
