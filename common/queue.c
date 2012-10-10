/*
 * queue.c
 *
 *  Created on: Apr 16, 2010
 *      Author: xen
 */

#include "queue.h"
/*----------------------------------------------------------------------------*/
void queueInit(struct Queue *q)
{
  q->floor = 0;
  q->ceil = 0;
  q->size = 0;
}
/*----------------------------------------------------------------------------*/
void queuePush(struct Queue *q, uint8_t x)
{
  if (q->size < QUEUE_SIZE)
  {
    q->data[q->ceil++] = x;
    q->size++;
    if (q->ceil == QUEUE_SIZE)
      q->ceil = 0;
  }
}
/*----------------------------------------------------------------------------*/
uint8_t queuePop(struct Queue *q)
{
  uint8_t tmp;
  if (q->size)
  {
    tmp = q->data[q->floor++];
    if (q->floor == QUEUE_SIZE)
      q->floor = 0;
    q->size--;
    return tmp;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
uint8_t queuePeek(struct Queue *q)
{
  return ((q->size) ? q->data[q->floor] : 0);
}
/*----------------------------------------------------------------------------*/
uint8_t queueSize(struct Queue *q)
{
  return q->size;
}
/*//---------------------------------------------------------------------------
uint8_t queueFull(struct Queue *q)
{
  return (q->size == QUEUE_LENGTH);
}
//---------------------------------------------------------------------------
uint8_t queueEmpty(struct Queue *q)
{
  return (q->size == 0);
}
*/
