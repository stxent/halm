/*
 * halm/platform/lpc/sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SDMMC_H_
#define HALM_PLATFORM_LPC_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/interrupt.h>
#include <halm/platform/lpc/sdmmc_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Sdmmc;

struct SdmmcConfig
{
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
};

struct Sdmmc
{
  struct SdmmcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptor */
  struct Dma *dma;
  /* External interrupt on data line 0 */
  struct Interrupt *finalizer;
  /* Data line 0 pin */
  struct Pin data0;
  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Data rate */
  uint32_t rate;
  /* Status of the last command */
  enum Result status;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SDMMC_H_ */
