/*
 * halm/platform/lpc/gen_2/can.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_GEN_2_CAN_H_
#define HALM_PLATFORM_LPC_GEN_2_CAN_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/gen_2/can_base.h>
#include <xcore/interface.h>
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
  PointerArray pool;
  /* Queue for received messages */
  PointerQueue rxQueue;
  /* Queue for transmitting messages */
  PointerQueue txQueue;
  /* Pointer to a memory region used as a message pool */
  void *poolBuffer;
  /* Desired baud rate */
  uint32_t rate;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_CAN_H_ */
