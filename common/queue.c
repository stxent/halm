/*
 * queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <queue.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_CAPACITY 16
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *queue, uint32_t width, uint16_t capacity)
{
  if (!capacity)
    capacity = DEFAULT_CAPACITY;
  queue->data = malloc(width * capacity);
  if (!queue->data)
    return E_MEMORY;

  queue->width = width;
  queue->capacity = capacity;
  queueClear(queue);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void queueDeinit(struct Queue *queue)
{
  free(queue->data);
}
/*----------------------------------------------------------------------------*/
void queuePeek(struct Queue *queue, void *element)
{
  assert(queue->size);

  memcpy(element, queue->data + queue->width * queue->floor, queue->width);
}
/*----------------------------------------------------------------------------*/
void queuePop(struct Queue *queue, void *element)
{
  assert(queue->size);

  memcpy(element, queue->data + queue->width * queue->floor, queue->width);
  if (++queue->floor == queue->capacity)
    queue->floor = 0;
  --queue->size;
}
/*----------------------------------------------------------------------------*/
void queuePush(struct Queue *queue, const void *element)
{
  assert(queue->size < queue->capacity);

  memcpy(queue->data + queue->width * queue->ceil, &element, queue->width);
  if (++queue->ceil == queue->capacity)
    queue->ceil = 0;
  ++queue->size;
}
