/*
 * halm/platform/numicro/can.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_CAN_H_
#define HALM_PLATFORM_NUMICRO_CAN_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Can;

struct CanConfig
{
  /** Optional: timer for a message time stamp generation. */
  struct Timer *timer;
  /** Mandatory: baud rate. */
  uint32_t rate;
  /** Optional: number of filtering rules. */
  size_t filters;
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
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_CAN_H_ */
