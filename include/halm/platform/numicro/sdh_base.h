/*
 * halm/platform/numicro/sdh_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_SDH_BASE_H_
#define HALM_PLATFORM_NUMICRO_SDH_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SdhBase;

struct SdhBaseConfig
{
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
  /* Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct SdhBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
  /* 4-bit bus indicator */
  bool wide;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t sdhGetClock(const struct SdhBase *);
uint32_t sdhGetDivider(const struct SdhBase *);
void sdhSetDivider(struct SdhBase *, uint32_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_SDH_BASE_H_ */
