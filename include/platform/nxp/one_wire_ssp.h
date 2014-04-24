/*
 * platform/nxp/one_wire_ssp.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ONE_WIRE_SSP_H_
#define ONE_WIRE_SSP_H_
/*----------------------------------------------------------------------------*/
#include <byte_queue.h>
#include <irq.h>
#include "ssp_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *OneWireSsp;
/*----------------------------------------------------------------------------*/
enum oneWireSspState
{
  OW_SSP_IDLE = 0,
  OW_SSP_RESET,
  OW_SSP_PRESENCE,
  OW_SSP_RECEIVE,
  OW_SSP_TRANSMIT,
  OW_SSP_ERROR
};
/*----------------------------------------------------------------------------*/
struct OneWireSspConfig
{
  /** Mandatory: pin used for serial data reception from the bus. */
  gpio_t miso;
  /** Mandatory: pin used for data transmission on the serial bus. */
  gpio_t mosi;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct OneWireSsp
{
  struct SspBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Pointer to an input buffer */
  uint8_t *rxBuffer;
  /* Address of the device */
  uint64_t address;
  /* Number of bytes to be transmitted */
  uint8_t left;
  /* Position in a receiving word and temporary buffer for this word */
  uint8_t bit, word;
  /* Output queue containing command, address and data */
  struct ByteQueue txQueue;

  /* Current interface state */
  enum oneWireSspState state;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* ONE_WIRE_SSP_H_ */
