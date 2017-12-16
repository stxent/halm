/*
 * halm/platform/nxp/gen_2/can.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_CAN_H_
#define HALM_PLATFORM_NXP_CAN_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/array.h>
#include <xcore/containers/queue.h>
#include <xcore/interface.h>
#include <halm/platform/nxp/gen_2/can_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Can;

struct CanConfig
{
  /** Optional: timer for a message time stamp generation. */
  void *timer;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: input queue size. */
  size_t rxBuffers;
  /** Mandatory: output queue size. */
  size_t txBuffers;
  /** Mandatory: receiver input. */
  PinNumber rx;
  /** Mandatory: transmitter output. */
  PinNumber tx;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct Can
{
  struct CanBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Timer for the time stamp generation */
  struct Timer *timer;

  /* Message pool */
  struct Array pool;
  /* Input and output queues */
  struct Queue rxQueue, txQueue;
  /* Pointer to a memory region used as a message pool */
  void *poolBuffer;
  /* Desired baud rate */
  uint32_t rate;
  /* Current interface mode */
  uint8_t mode;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SERIAL_H_ */
