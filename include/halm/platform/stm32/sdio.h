/*
 * halm/platform/stm32/sdio.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SDIO_H_
#define HALM_PLATFORM_STM32_SDIO_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/sdio_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Sdio;

struct Dma;
struct Interrupt;
struct Timer;

struct SdioConfig
{
  /** Optional: timer for data timeout calculation. */
  void *timer;
  /** Mandatory: data rate. */
  uint32_t rate;
  /** Mandatory: clock line. */
  PinNumber clk;
  /** Mandatory: command line. */
  PinNumber cmd;
  /** Mandatory: data line 0. */
  PinNumber dat0;
  /** Optional: data line 1. */
  PinNumber dat1;
  /** Optional: data line 2. */
  PinNumber dat2;
  /** Optional: data line 3. */
  PinNumber dat3;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: number of the DMA stream. */
  uint8_t dma;
};

struct Sdio
{
  struct SdioBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptor for data reception */
  struct Dma *rxDma;
  /* DMA channel descriptor for data transmission */
  struct Dma *txDma;
  /* External interrupt on data line 0 */
  struct Interrupt *finalizer;
  /* Timer for data timeout calculation */
  struct Timer *timer;
  /* Data line 0 pin */
  struct Pin data0;

  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Data transfer length */
  uint32_t count;
  /* Data rate */
  uint32_t rate;
  /* Data timeout */
  uint32_t timeout;
  /* Status of the current command */
  enum Result cmdStatus;
  /* Status of the DMA transfer */
  enum Result dmaStatus;
  /* Block size as a power of two */
  uint8_t block;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SDIO_H_ */
