/*
 * platform/nxp/sdmmc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_SDMMC_BASE_H_
#define PLATFORM_NXP_SDMMC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <irq.h>
#include <pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SdmmcBase;
/*----------------------------------------------------------------------------*/
struct SdmmcBaseConfig
{
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
struct SdmmcBase
{
  struct Entity parent;

  void *reg;
  void (*handler)(void *);
  irq_t irq;

  /* 4-bit bus indicator */
  bool wide;
};
/*----------------------------------------------------------------------------*/
uint32_t sdmmcGetClock(const struct SdmmcBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_SDMMC_BASE_H_ */
