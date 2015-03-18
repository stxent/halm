/*
 * platform/nxp/sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SDMMC_H_
#define PLATFORM_NXP_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <interface.h>
#include <irq.h>
#include <pin.h>
#include <platform/nxp/pin_interrupt.h>
#include <platform/nxp/sdmmc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Sdmmc;
/*----------------------------------------------------------------------------*/
struct SdmmcConfig
{
  /** Mandatory: data rate. */
  uint32_t rate;
  /** Mandatory: clock line. */
  pin_t clk;
  /** Mandatory: command line. */
  pin_t cmd;
  /** Mandatory: data line 0. */
  pin_t dat0;
  /** Optional: data line 1. */
  pin_t dat1;
  /** Optional: data line 2. */
  pin_t dat2;
  /** Optional: data line 3. */
  pin_t dat3;
};
/*----------------------------------------------------------------------------*/
struct Sdmmc
{
  struct SdmmcBase parent;

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
#endif /* PLATFORM_NXP_SDMMC_H_ */
