/*
 * interface.h
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

/**
 * @file
 * Abstract peripheral interface for embedded system applications.
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include "entity.h"
#include "error.h"
/*----------------------------------------------------------------------------*/
/** Interface options. */
enum ifOption
{
  /** Device address. */
  IF_DEVICE,
  /** Sub-address. */
  IF_ADDRESS,
  /** Data rate. */
  IF_RATE,
  /** Non-blocking operations mode. */
  IF_NONBLOCKING,
  /** Interface interrupts or DMA requests priority. */
  IF_PRIORITY,
  /** Check whether interface is busy. */
  IF_BUSY,
  /** Size of the total space available on device. */
  IF_SIZE
};
/*----------------------------------------------------------------------------*/
struct InterfaceClass
{
  CLASS_GENERATOR

  uint32_t (*read)(void *, uint8_t *, uint32_t);
  uint32_t (*write)(void *, const uint8_t *, uint32_t);
  enum result (*get)(void *, enum ifOption, void *);
  enum result (*set)(void *, enum ifOption, const void *);
};
/*----------------------------------------------------------------------------*/
struct Interface
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
uint32_t ifRead(void *, uint8_t *, uint32_t);
uint32_t ifWrite(void *, const uint8_t *, uint32_t);
enum result ifGet(void *, enum ifOption, void *);
enum result ifSet(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
#endif /* INTERFACE_H_ */
