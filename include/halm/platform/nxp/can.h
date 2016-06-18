/*
 * halm/platform/nxp/can.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_CAN_H_
#define HALM_PLATFORM_NXP_CAN_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/queue.h>
#include <xcore/interface.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_CAN/can_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Can;
/*----------------------------------------------------------------------------*/
struct CanConfig
{
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Mandatory: input queue size. */
  size_t rxBuffers;
  /** Mandatory: output queue size. */
  size_t txBuffers;
  /** Mandatory: receiver input. */
  pinNumber rx;
  /** Mandatory: transmitter output. */
  pinNumber tx;
  /** Optional: interrupt priority. */
  irqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct Can
{
  struct CanBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Message pool */
  struct Queue pool;
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
