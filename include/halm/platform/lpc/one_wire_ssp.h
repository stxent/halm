/*
 * halm/platform/lpc/one_wire_ssp.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ONE_WIRE_SSP_H_
#define HALM_PLATFORM_LPC_ONE_WIRE_SSP_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/one_wire.h>
#include <halm/irq.h>
#include <halm/platform/lpc/ssp_base.h>
#include <xcore/containers/byte_queue.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const OneWireSsp;

struct OneWireSspConfig
{
  /** Mandatory: pin used for serial data reception from the bus. */
  PinNumber miso;
  /** Mandatory: pin used for data transmission on the serial bus. */
  PinNumber mosi;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct OneWireSsp
{
  struct SspBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Address of the device */
  uint64_t address;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Output queue containing command, address and data */
  struct ByteQueue txQueue;

  /* Number of words to be transmitted */
  uint8_t left;
  /* Position in a receiving word */
  uint8_t bit;
  /* Temporary buffer for receiving word */
  uint8_t word;

  /* Current interface state */
  uint8_t state;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;

  /* Temporary variables for device search algorithm */
  uint8_t lastDiscrepancy, lastZero;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_ONE_WIRE_SSP_H_ */
