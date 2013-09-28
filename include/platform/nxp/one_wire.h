/*
 * one_wire.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_
/*----------------------------------------------------------------------------*/
#include "queue.h"
#include "platform/nxp/spinlock.h"
#include "./nvic.h"
#include "./uart.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *OneWire;
/*----------------------------------------------------------------------------*/
struct OneWireConfig
{
  uint32_t rxLength, txLength; /* Optional: queue lengths */
  gpioKey rx, tx; /* Mandatory: RX and TX pins */
  uint8_t channel; /* Mandatory: Peripheral number */
};
/*----------------------------------------------------------------------------*/
struct OneWireAddress
{
  union
  {
    uint64_t rom;
    struct
    {
      uint8_t family;
      uint8_t serial[6];
      uint8_t crc;
    };
  };
};
/*----------------------------------------------------------------------------*/
enum oneWireState
{
  OW_IDLE = 0,
  OW_RESET,
  OW_READY,
  OW_TRANSMIT,
  OW_RECEIVE
};
/*----------------------------------------------------------------------------*/
struct OneWire
{
  struct Uart parent;

  /* Pointer to the callback function and to the callback argument */
  void (*callback)(void *);
  void *callbackArgument;

  /* Receive buffer */
  uint8_t *rxBuffer;
  /* Address of the device */
  struct OneWireAddress address;
  /* Number of bytes to be transmitted */
  uint8_t left;
  /* Position in the receiving word and temporary buffer for the word */
  uint8_t bit, word;
  /* Transmit queue */
  struct Queue txQueue;

  /* Computed data rates for reset sequence and for data transmission */
  struct UartRateConfig dataRate, resetRate;

  /* Current 1-Wire interface state */
  enum oneWireState state;
  /* Exclusive access to channel */
  spinlock_t lock;
  /* Selection between blocking mode and zero copy mode */
  bool blocking;
};
/*----------------------------------------------------------------------------*/
#endif /* ONE_WIRE_H_ */
