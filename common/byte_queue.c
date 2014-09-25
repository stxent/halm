/*
 * byte_queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <byte_queue.h>
/*----------------------------------------------------------------------------*/
enum result byteQueueInit(struct ByteQueue *queue, unsigned int capacity)
{
  if (!capacity || capacity > USHRT_MAX)
    return E_VALUE;

  queue->data = malloc(capacity);
  if (!queue->data)
    return E_MEMORY;

  queue->capacity = (unsigned short)capacity;
  byteQueueClear(queue);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void byteQueueDeinit(struct ByteQueue *queue)
{
  free(queue->data);
}
/*----------------------------------------------------------------------------*/
unsigned int byteQueuePopArray(struct ByteQueue *queue, uint8_t *buffer,
    unsigned int length)
{
  unsigned short count, moved = 0;

  if (!queue->size)
    return 0;

  if (queue->ceil <= queue->floor)
  {
    count = queue->capacity - queue->floor;

    if (length < count)
      count = length;

    if (count)
    {
      memcpy(buffer, queue->data + queue->floor, count);
      queue->floor += count;
      if (queue->floor == queue->capacity)
        queue->floor = 0;
      queue->size -= count;

      buffer += count;
      moved += count;
      length -= count;
    }
  }

  if (queue->ceil > queue->floor)
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

  return (unsigned int)moved;
}
/*----------------------------------------------------------------------------*/
unsigned int byteQueuePushArray(struct ByteQueue *queue, const uint8_t *buffer,
    unsigned int length)
{
  unsigned short count, moved = 0;

  if (queue->ceil >= queue->floor)
  {
    count = queue->capacity - queue->ceil;

    if (length < count)
      count = length;

    if (count)
    {
      memcpy(queue->data + queue->ceil, buffer, count);
      queue->ceil = queue->ceil + count;
      if (queue->ceil == queue->capacity)
        queue->ceil = 0;
      queue->size += count;

      buffer += count;
      moved += count;
      length -= count;
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

  return (unsigned int)moved;
}
