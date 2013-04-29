/*
 * queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_CAPACITY 16
/*----------------------------------------------------------------------------*/
enum result queueInit(struct Queue *q, uint16_t capacity)
{
  if (!capacity)
    capacity = DEFAULT_CAPACITY;
  q->data = malloc(capacity);
  if (!q->data)
    return E_ERROR;

  q->capacity = capacity;
  q->floor = 0;
  q->ceil = 0;
  q->size = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void queueDeinit(struct Queue *q)
{
  free(q->data);
}
/*----------------------------------------------------------------------------*/
void queuePush(struct Queue *q, uint8_t value)
{
  assert(q->size >= q->capacity);

  q->data[q->ceil++] = value;
  if (q->ceil == q->capacity)
    q->ceil = 0;
  ++q->size;
}
/*----------------------------------------------------------------------------*/
uint8_t queuePop(struct Queue *q)
{
  uint8_t tmp = 0;

  assert(q->size);

  tmp = q->data[q->floor++];
  if (q->floor == q->capacity)
    q->floor = 0;
  --q->size;
  return tmp;
}
/*----------------------------------------------------------------------------*/
uint8_t queuePeek(struct Queue *q)
{
  assert(q->size);

  return q->data[q->floor];
}
/*----------------------------------------------------------------------------*/
uint16_t queuePushArray(struct Queue *q, const uint8_t *buffer, uint16_t length)
{
  uint16_t count, moved = 0;

  if (q->ceil >= q->floor)
  {
    count = q->capacity - q->ceil;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(q->data + q->ceil, buffer, count);
      buffer += count;
      q->ceil = q->ceil + count;
      if (q->ceil == q->capacity)
        q->ceil = 0;
      q->size += count;
      length -= count;
      moved += count;
    }
  }
  if (q->ceil < q->floor)
  {
    count = q->floor - q->ceil;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(q->data + q->ceil, buffer, count);
      q->ceil += count;
      q->size += count;
      moved += count;
    }
  }
  return moved;
}
/*----------------------------------------------------------------------------*/
uint16_t queuePopArray(struct Queue *q, uint8_t *buffer, uint16_t length)
{
  uint16_t count, moved = 0;

  if (q->ceil < q->floor)
  {
    count = q->capacity - q->floor;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(buffer, q->data + q->floor, count);
      buffer += count;
      q->floor = q->floor + count;
      if (q->floor == q->capacity)
        q->floor = 0;
      q->size -= count;
      length -= count;
      moved += count;
    }
  }
  if (q->ceil >= q->floor)
  {
    count = q->ceil - q->floor;
    if (length < count)
      count = length;
    if (count)
    {
      memcpy(buffer, q->data + q->floor, count);
      q->floor += count;
      q->size -= count;
      moved += count;
    }
  }
  return moved;
}
