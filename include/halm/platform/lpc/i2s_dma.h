/*
 * halm/platform/lpc/i2s_dma.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_I2S_DMA_H_
#define HALM_PLATFORM_LPC_I2S_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/lpc/i2s_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2SDma;

struct I2SDmaConfig
{
  /** Mandatory: sample rate for the receiver and the transmitter. */
  uint32_t rate;
  /** Mandatory: word width. */
  enum I2SWidth width;

  struct
  {
    /** Mandatory: receive data. */
    PinNumber sda;
    /** Optional: receive clock. */
    PinNumber sck;
    /** Optional: receive word select. */
    PinNumber ws;
    /** Optional: master clock output. */
    PinNumber mclk;
    /** Mandatory: direct memory access channel. */
    uint8_t dma;
  } rx;

  struct
  {
    /** Mandatory: transmit data. */
    PinNumber sda;
    /** Optional: transmit clock. */
    PinNumber sck;
    /** Optional: transmit word select. */
    PinNumber ws;
    /** Optional: master clock output. */
    PinNumber mclk;
    /** Mandatory: direct memory access channel. */
    uint8_t dma;
  } tx;

  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: enable mono format. */
  bool mono;
  /** Optional: enable slave mode. */
  bool slave;
};

struct I2SDma
{
  struct I2SBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Direct memory access channel descriptors */
  struct Dma *rxDma, *txDma;
  /* Sample rate */
  uint32_t sampleRate;
  /* Size of the sample in power of two format */
  uint8_t sampleSize;
  /* Enable mono mode */
  bool mono;
  /* Enable slave mode */
  bool slave;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_I2S_DMA_H_ */
