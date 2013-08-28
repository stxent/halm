/*
 * one_wire.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_
/*----------------------------------------------------------------------------*/
#include "queue.h"
#include "threading/mutex.h"
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

  void (*callback)(void *);
  void *callbackArgument;

  uint8_t *rxBuffer; /* Receive buffer */

  struct OneWireAddress address; /* Currently selected device */
  uint8_t rxPosition, left;
  uint8_t word;

  struct Queue txQueue; /* Transmit queue */
  struct UartRateConfig dataRate, resetRate;
  bool blocking; /* By default interface is in blocking mode */
  struct Mutex channelLock; /* Access to base interface */
  enum oneWireState state; /* Current 1-Wire interface state */
};
/*----------------------------------------------------------------------------*/
#endif /* ONE_WIRE_H_ */
