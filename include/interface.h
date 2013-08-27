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
  /** Bytes available in receive buffers. */
  IF_AVAILABLE,
  /** Check whether interface is busy. */
  IF_BUSY,
  /** Bytes pending in transmit buffers. */
  IF_PENDING,
  /** Size of the total space available on device. */
  IF_SIZE,

  /** Sub-address. */
  IF_ADDRESS,
  /** Burst size for scatter-gather operations. */
  IF_BURST,
  /** Device address. */
  IF_DEVICE,
  /** Interface lock. */
  IF_LOCK,
  /** Non-blocking mode with copying into internal buffers. */
  IF_NONBLOCKING,
  /** Interface interrupts or DMA requests priority. */
  IF_PRIORITY,
  /** Data rate. */
  IF_RATE,
  /** Timeout value for blocking functions. */
  IF_TIMEOUT,
  /** Zero-copy mode. */
  IF_ZEROCOPY
};
/*----------------------------------------------------------------------------*/
struct InterfaceClass
{
  CLASS_HEADER

  enum result (*callback)(void *, void (*)(void *), void *);
  enum result (*get)(void *, enum ifOption, void *);
  enum result (*set)(void *, enum ifOption, const void *);
  uint32_t (*read)(void *, uint8_t *, uint32_t);
  uint32_t (*write)(void *, const uint8_t *, uint32_t);
};
/*----------------------------------------------------------------------------*/
struct Interface
{
  struct Entity parent;
};
/*----------------------------------------------------------------------------*/
enum result ifCallback(void *, void (*)(void *), void *);
enum result ifGet(void *, enum ifOption, void *);
enum result ifSet(void *, enum ifOption, const void *);
uint32_t ifRead(void *, uint8_t *, uint32_t);
uint32_t ifWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
#endif /* INTERFACE_H_ */
