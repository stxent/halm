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
  /** Bytes available in the receive buffer. */
  IF_AVAILABLE,
  /** Bytes pending in the transmit buffer. */
  IF_PENDING,

  /** Burst size for the scatter-gather operations. */
  IF_BURST,
  /** Address of the device. */
  IF_DEVICE,
  /** Priority of the interrupts or the memory requests. */
  IF_PRIORITY,
  /** Data rate. */
  IF_RATE,
  /** Timeout value for blocking functions. */
  IF_TIMEOUT,

  /** Internal address of the device.
   * Some devices can use 64-bit unsigned integer as an argument.
   */
  IF_ADDRESS,
  /** Total memory available for use with the internal addressing method. */
  IF_SIZE,

  /** Check whether the interface is ready to the next portion of data.
   * Returns @b E_OK when interface is ready or @b E_BUSY otherwise.
   */
  IF_READY,

  /** Select blocking mode for the interface. */
  IF_BLOCKING,
  /** Select non-blocking mode for the interface. */
  IF_NONBLOCKING,
  /** Select zero-copy mode for the interface. */
  IF_ZEROCOPY,

  /** Acquire the interface. Returns @b E_OK on success or @b E_BUSY. */
  IF_ACQUIRE,
  /** Release the interface. */
  IF_RELEASE
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
