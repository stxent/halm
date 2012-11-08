/*
 * queue.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef QUEUE_H_
#define QUEUE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#ifndef QUEUE_SIZE
#define QUEUE_SIZE 32
#endif /* QUEUE_SIZE */
/*----------------------------------------------------------------------------*/
struct Queue
{
  uint8_t data[QUEUE_SIZE];
  uint8_t floor, ceil, size;
};
/*----------------------------------------------------------------------------*/
void queueInit(struct Queue *);
void queuePush(struct Queue *, uint8_t);
uint8_t queuePop(struct Queue *);
uint8_t queuePeek(struct Queue *);
uint8_t queueSize(struct Queue *);
/*uint8_t queueFull(struct Queue *);
uint8_t queueEmpty(struct Queue *);*/
/*----------------------------------------------------------------------------*/
#endif /* QUEUE_H_ */
