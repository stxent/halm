/*
 * halm/platform/nxp/sdmmc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SDMMC_BASE_H_
#define HALM_PLATFORM_NXP_SDMMC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SdmmcBase;
/*----------------------------------------------------------------------------*/
struct SdmmcBaseConfig
{
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
};
/*----------------------------------------------------------------------------*/
struct SdmmcBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;

  /* 4-bit bus indicator */
  bool wide;
};
/*----------------------------------------------------------------------------*/
uint32_t sdmmcGetClock(const struct SdmmcBase *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SDMMC_BASE_H_ */
