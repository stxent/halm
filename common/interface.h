/*
 * interface.h
 *
 *  Created on: Oct 20, 2012
 *      Author: xen
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_
/*------------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------------------------------------------------------------------*/
#include "entity.h"
#include "mutex.h"
/*------------------------------------------------------------------------------*/
typedef uint32_t memSize; /* Type for buffers size */
typedef uint64_t memAddress; /* Type for addresses */
/*------------------------------------------------------------------------------*/
enum ifOption {
  IF_SPEED = 0,
  IF_BUFFER_SIZE,
  IF_SYNC,
  IF_QUEUE_RX,
  IF_QUEUE_TX
};
/*------------------------------------------------------------------------------*/
extern const struct EntityType *Interface;
/*------------------------------------------------------------------------------*/
struct Interface
{
  struct Entity parent;
  struct Mutex lock;
  /* Start transmission, arguments: device address */
  int (*start)(struct Interface *, uint8_t *);
  /* Stop transmission */
  void (*stop)(struct Interface *);
  /* Receive data, arguments: data buffer, message size */
  memSize (*read)(struct Interface *, uint8_t *, memAddress, memSize);
  /* Send data, arguments: data buffer, message size */
  memSize (*write)(struct Interface *, const uint8_t *, memAddress, memSize);
  /* Get interface option, arguments: option id, pointer to save value */
  int (*getopt)(struct Interface *, enum ifOption, void *);
  /* Set interface option, arguments: option id, pointer to new value */
  int (*setopt)(struct Interface *, enum ifOption, const void *);
};
/*------------------------------------------------------------------------------*/
int ifStart(struct Interface *, uint8_t *);
void ifStop(struct Interface *);
memSize ifRead(struct Interface *, uint8_t *, memAddress, memSize);
memSize ifWrite(struct Interface *, const uint8_t *, memAddress, memSize);
memSize ifBlockRead(struct Interface *, uint8_t *, memAddress, memSize);
memSize ifBlockWrite(struct Interface *, const uint8_t *, memAddress, memSize);
int ifGetOpt(struct Interface *, enum ifOption, void *);
int ifSetOpt(struct Interface *, enum ifOption, const void *);
/*------------------------------------------------------------------------------*/
#endif /* INTERFACE_H_ */
