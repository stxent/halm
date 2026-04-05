/*
 * halm/platform/stm32/i2s_dma.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_I2S_DMA_H_
#define HALM_PLATFORM_STM32_I2S_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/stm32/spi_base.h>
#include <xcore/stream.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const I2SDma;

struct I2SDmaStream;

enum [[gnu::packed]] I2SWidth
{
  I2S_WIDTH_16,
  I2S_WIDTH_32
};

struct I2SDmaConfig
{
  /** Mandatory: DMA buffer size in bytes. */
  size_t depth;
  /** Mandatory: request queue size. */
  size_t size;
  /** Mandatory: sample rate for the receiver and the transmitter. */
  uint32_t rate;
  /** Optional: DMA priority. Low priority is used by default. */
  enum DmaPriority priority;
  /** Mandatory: word width. */
  enum I2SWidth width;

  struct
  {
    /** Mandatory: receive data. */
    PinNumber sd;
    /** Mandatory: direct memory access channel. */
    uint8_t dma;
  } rx;

  struct
  {
    /** Mandatory: transmit data. */
    PinNumber sd;
    /** Mandatory: direct memory access channel. */
    uint8_t dma;
  } tx;

  /** Optional: master clock output. */
  PinNumber mck;
  /** Mandatory: receive clock. */
  PinNumber sck;
  /** Mandatory: receive word select. */
  PinNumber ws;

  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: enable slave mode. */
  bool slave;
};

struct I2SDma
{
  struct SpiBase base;

  /* Input stream */
  struct I2SDmaStream *rxStream;
  /* Output stream */
  struct I2SDmaStream *txStream;
  /* DMA channel descriptors for receiving data */
  struct Dma *rxDma;
  /* DMA channel descriptors for transmitting data */
  struct Dma *txDma;

  /* DMA buffer size */
  size_t bufferSize;
  /* Sample rate */
  uint32_t sampleRate;
  /* Size of the sample in power of two format */
  uint8_t sampleSize;
  /* Enable full duplex mode */
  bool fullduplex;
  /* Enable slave mode */
  bool slave;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Stream *i2sDmaGetInput(struct I2SDma *);
struct Stream *i2sDmaGetOutput(struct I2SDma *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_I2S_DMA_H_ */
