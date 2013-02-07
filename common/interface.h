/*
 * interface.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "entity.h"
#include "error.h"
/*----------------------------------------------------------------------------*/
enum ifOption {
  IF_ADDRESS,       /* Sub-address */
  IF_SPEED,         /* Data rate */
  IF_BLOCKING,      /* Blocking operations mode */
  IF_NONBLOCKING,   /* Non-blocking operations mode */
  IF_PRIORITY,      /* Interface interrupts or DMA requests priority */
  IF_QUEUE_RX,
  IF_QUEUE_TX
};
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct InterfaceClass
{
  CLASS_GENERATOR

  /* Receive data, arguments: data buffer, message size */
  uint32_t (*read)(void *, uint8_t *, uint32_t);
  /* Send data, arguments: data buffer, message size */
  uint32_t (*write)(void *, const uint8_t *, uint32_t);
  /* Get interface option, arguments: option id, pointer to save value */
  enum result (*getopt)(void *, enum ifOption, void *);
  /* Set interface option, arguments: option id, pointer to new value */
  enum result (*setopt)(void *, enum ifOption, const void *);
};
/*----------------------------------------------------------------------------*/
struct Interface
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
uint32_t ifRead(void *, uint8_t *, uint32_t);
uint32_t ifWrite(void *, const uint8_t *, uint32_t);
enum result ifGetOpt(void *, enum ifOption, void *);
enum result ifSetOpt(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
#endif /* INTERFACE_H_ */
