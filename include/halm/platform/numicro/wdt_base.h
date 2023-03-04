/*
 * halm/platform/numicro/wdt_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_WDT_BASE_H_
#define HALM_PLATFORM_NUMICRO_WDT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const WdtBase;

struct WdtBase
{
  struct Entity base;

  void (*handler)(void *);
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t wdtGetClock(const struct WdtBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_WDT_BASE_H_ */
