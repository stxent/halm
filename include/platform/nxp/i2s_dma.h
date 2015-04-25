/*
 * platform/nxp/i2s_dma.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_I2S_DMA_H_
#define PLATFORM_NXP_I2S_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <platform/nxp/i2s_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2sDma;
/*----------------------------------------------------------------------------*/
struct I2sDmaConfig
{
  /** Mandatory: sample rate for the receiver and the transmitter. */
  uint32_t rate;
  /** Mandatory: size of the single buffer in elements. */
  uint32_t size;

  struct
  {
    /** Optional: receive clock. */
    pin_t sck;
    /** Optional: receive word select. */
    pin_t ws;
    /** Optional: receive data. */
    pin_t sda;
    /** Optional: master clock output. */
    pin_t mclk;
  } rx;

  struct
  {
    /** Optional: transmit clock. */
    pin_t sck;
    /** Optional: transmit word select. */
    pin_t ws;
    /** Optional: transmit data. */
    pin_t sda;
    /** Optional: master clock output. */
    pin_t mclk;
  } tx;

  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: word width. */
  enum i2sWidth width;
  /** Mandatory: enable stereo format. */
  bool stereo;
};
/*----------------------------------------------------------------------------*/
struct I2sDma
{
  struct I2sBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Direct memory access channel descriptor */
  struct Dma *dma;
  /* Sample rate */
  uint32_t sampleRate;
  /* Size of each buffer */
  uint32_t size;
  /* Word width */
  enum i2sWidth width;
  /* Stereo format enabled flag */
  bool stereo;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_I2S_DMA_H_ */
