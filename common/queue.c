/*
 * queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
#include <stdlib.h>
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
  if (q->size < q->capacity)
  {
    q->data[q->ceil++] = value;
    if (q->ceil == q->capacity)
      q->ceil = 0;
    q->size++;
  }
}
/*----------------------------------------------------------------------------*/
uint8_t queuePop(struct Queue *q)
{
  uint8_t tmp = 0;

  if (q->size)
  {
    tmp = q->data[q->floor++];
    if (q->floor == q->capacity)
      q->floor = 0;
    q->size--;
  }
  return tmp;
}
/*----------------------------------------------------------------------------*/
uint8_t queuePeek(struct Queue *q)
{
  return (q->size) ? q->data[q->floor] : 0;
}
/*----------------------------------------------------------------------------*/
void queuePushArray(struct Queue *q, const uint8_t *buffer, uint16_t length)
{
  uint16_t len;

  if (q->ceil >= q->floor)
  {
    len = q->capacity - q->ceil;
    if (length < len)
      len = length;
    if (len)
    {
      memcpy(q->data + q->ceil, buffer, len);
      buffer += len;
      q->ceil = q->ceil + len;
      if (q->ceil == q->capacity)
        q->ceil = 0;
      q->size += len;
      length -= len;
    }
  }
  if (q->ceil < q->floor)
  {
    len = q->floor - q->ceil;
    if (length < len)
      len = length;
    if (len)
    {
      memcpy(q->data + q->ceil, buffer, len);
      q->ceil += len;
      q->size += len;
    }
  }
}
/*----------------------------------------------------------------------------*/
void queuePopArray(struct Queue *q, uint8_t *buffer, uint16_t length)
{
  uint16_t len;

  if (q->ceil <= q->floor)
  {
    len = q->capacity - q->floor;
    if (length < len)
      len = length;
    if (len)
    {
      memcpy(buffer, q->data, len);
      buffer += len;
      q->floor = q->floor + len;
      if (q->floor == q->capacity)
        q->floor = 0;
      q->size -= len;
      length -= len;
    }
  }
  if (q->ceil > q->floor)
  {
    len = q->ceil - q->floor;
    if (length < len)
      len = length;
    if (len)
    {
      memcpy(buffer, q->data + q->floor, len);
      q->floor += len;
      q->size -= len;
    }
  }
}
