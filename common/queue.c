/*
 * queue.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
/*----------------------------------------------------------------------------*/
#include "queue.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_CAPACITY    16
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
void queuePush(struct Queue *q, uint8_t x)
{
  if (q->size < q->capacity)
  {
    q->data[q->ceil++] = x;
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
  return ((q->size) ? q->data[q->floor] : 0);
}
/*----------------------------------------------------------------------------*/
/* uint16_t queueSize(struct Queue *q)
{
  return q->size;
} */
/*----------------------------------------------------------------------------*/
/* bool queueFull(struct Queue *q)
{
  return q->size == q->capacity;
} */
/*----------------------------------------------------------------------------*/
/* bool queueEmpty(struct Queue *q)
{
  return !q->size;
} */
