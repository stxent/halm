/*
 * queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *queue, unsigned int width,
    unsigned int capacity)
{
  /* Resulting capacity is lower than the maximum possible input value */
  if (!capacity || capacity > USHRT_MAX)
    return E_VALUE;

  queue->data = malloc(width * capacity);
  if (!queue->data)
    return E_MEMORY;

  queue->width = width;
  queue->capacity = (unsigned short)capacity;
  queueClear(queue);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void queueDeinit(struct Queue *queue)
{
  free(queue->data);
}
/*----------------------------------------------------------------------------*/
void queuePeek(const struct Queue *queue, void *element)
{
  assert(queue->size);

  memcpy(element, (char *)queue->data + queue->width * queue->floor,
      queue->width);
}
/*----------------------------------------------------------------------------*/
void queuePop(struct Queue *queue, void *element)
{
  assert(queue->size);

  memcpy(element, (char *)queue->data + queue->width * queue->floor,
      queue->width);

  if (++queue->floor == queue->capacity)
    queue->floor = 0;

  --queue->size;
}
/*----------------------------------------------------------------------------*/
void queuePush(struct Queue *queue, const void *element)
{
  assert(queue->size < queue->capacity);

  memcpy((char *)queue->data + queue->width * queue->ceil, &element,
      queue->width);

  if (++queue->ceil == queue->capacity)
    queue->ceil = 0;

  ++queue->size;
}
