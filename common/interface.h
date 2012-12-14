/*
 * interface.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_
/*------------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------------------------------------------------------------------*/
#include "error.h"
#include "entity.h"
/*------------------------------------------------------------------------------*/
enum ifOption {
  IF_ADDRESS,       /* Sub-address */
  IF_SPEED,         /* Data rate */
  IF_BUFFER_SIZE,
  IF_SYNC,          /* Synchronous or asynchronous mode */
  IF_QUEUE_RX,
  IF_QUEUE_TX
};
/*----------------------------------------------------------------------------*/
struct Interface;
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct InterfaceClass
{
  CLASS_GENERATOR(Interface)

  /* Receive data, arguments: data buffer, message size */
  uint32_t (*read)(struct Interface *, uint8_t *, uint32_t);
  /* Send data, arguments: data buffer, message size */
  uint32_t (*write)(struct Interface *, const uint8_t *, uint32_t);
  /* Get interface option, arguments: option id, pointer to save value */
  enum result (*getopt)(struct Interface *, enum ifOption, void *);
  /* Set interface option, arguments: option id, pointer to new value */
  enum result (*setopt)(struct Interface *, enum ifOption, const void *);
};
/*----------------------------------------------------------------------------*/
struct Interface
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
uint32_t ifRead(struct Interface *, uint8_t *, uint32_t);
uint32_t ifWrite(struct Interface *, const uint8_t *, uint32_t);
enum result ifGetOpt(struct Interface *, enum ifOption, void *);
enum result ifSetOpt(struct Interface *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
#endif /* INTERFACE_H_ */
