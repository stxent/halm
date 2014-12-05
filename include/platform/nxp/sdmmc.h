/*
 * platform/nxp/sdmmc.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SDMMC_H_
#define PLATFORM_NXP_SDMMC_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <irq.h>
#include <pin.h>
#include <platform/nxp/sdmmc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Sdmmc;
/*----------------------------------------------------------------------------*/
struct SdmmcConfig
{
  /** Mandatory: clock line. */
  pin_t clock;
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
struct SdmmcDmaDescriptor
{
  uint32_t control;
  uint32_t size;
  uint32_t buffer1;
  uint32_t buffer2;
};
/*----------------------------------------------------------------------------*/
struct Sdmmc
{
  struct SdmmcBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Argument for the most recent command */
  uint32_t argument;
  /* Interface command */
  uint32_t command;
  /* Data rate */
  uint32_t rate;
  /* Status of the last command */
  enum result status;

  struct SdmmcDmaDescriptor descriptor[16];

  struct Pin debug;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDMMC_H_ */
