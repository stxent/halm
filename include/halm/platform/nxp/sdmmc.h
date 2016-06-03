/*
 * halm/platform/nxp/sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SDMMC_H_
#define HALM_PLATFORM_NXP_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/dma.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/platform/nxp/pin_interrupt.h>
#include <halm/platform/nxp/sdmmc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Sdmmc;
/*----------------------------------------------------------------------------*/
struct SdmmcConfig
{
  /** Mandatory: data rate. */
  uint32_t rate;
  /** Mandatory: clock line. */
  pinNumber clk;
  /** Mandatory: command line. */
  pinNumber cmd;
  /** Mandatory: data line 0. */
  pinNumber dat0;
  /** Optional: data line 1. */
  pinNumber dat1;
  /** Optional: data line 2. */
  pinNumber dat2;
  /** Optional: data line 3. */
  pinNumber dat3;
  /** Optional: interrupt priority. */
  irqPriority priority;
};
/*----------------------------------------------------------------------------*/
struct Sdmmc
{
  struct SdmmcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel descriptor */
  struct Dma *dma;
  /* External interrupt on data line 0 */
  struct Interrupt *finalizer;
  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Data rate */
  uint32_t rate;
  /* Status of the last command */
  enum result status;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SDMMC_H_ */
