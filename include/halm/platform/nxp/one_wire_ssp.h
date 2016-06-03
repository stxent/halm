/*
 * halm/platform/nxp/one_wire_ssp.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_ONE_WIRE_SSP_H_
#define HALM_PLATFORM_NXP_ONE_WIRE_SSP_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/byte_queue.h>
#include <halm/common/one_wire.h>
#include <halm/irq.h>
#include <halm/platform/nxp/ssp_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const OneWireSsp;
/*----------------------------------------------------------------------------*/
struct OneWireSspConfig
{
  /** Mandatory: pin used for serial data reception from the bus. */
  pinNumber miso;
  /** Mandatory: pin used for data transmission on the serial bus. */
  pinNumber mosi;
  /** Optional: interrupt priority. */
  irqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
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
#endif /* HALM_PLATFORM_NXP_ONE_WIRE_SSP_H_ */
