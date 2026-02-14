/*
 * halm/platform/numicro/qspi.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_QSPI_H_
#define HALM_PLATFORM_NUMICRO_QSPI_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/qspi.h>
#include <halm/platform/numicro/pdma_base.h>
#include <halm/platform/numicro/qspi_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Qspi;

struct QspiConfig
{
  /** Mandatory: serial data rate. */
  uint32_t rate;
  /** Optional: slave select pin. */
  PinNumber cs;
  /**
   * Optional: data output in single-wire mode, input/output pin 0
   * in dual or quad mode.
   */
  PinNumber io0;
  /**
   * Optional: data input in single-wire mode, input/output pin 1
   * in dual or quad mode.
   */
  PinNumber io1;
  /** Optional: input/output pin 2 in quad mode. */
  PinNumber io2;
  /** Optional: input/output pin 3 in quad mode. */
  PinNumber io3;
  /** Optional: serial clock output. */
  PinNumber sck;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: mode number. */
  uint8_t mode;
  /**
   * Mandatory: DMA channels used for incoming and outgoing data, the channel
   * with higher priority will be used for reception.
   */
  uint8_t dma[2];
};

struct Qspi
{
  struct QspiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Desired baud rate */
  uint32_t rate;

  /* DMA descriptor for RX channel */
  struct Dma *rxDma;
  /* DMA descriptor for TX channel */
  struct Dma *txDma;

  /*
   * Dummy frame to be sent over transmit line in the receive mode or
   * to be received in the transmit mode.
   */
  uint8_t dummy;

  /* Selection between blocking mode and zero copy mode */
  bool blocking;
  /* Synchronize DMA TX and RX handlers */
  bool invoked;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_QSPI_H_ */
