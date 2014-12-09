/*
 * wwdt_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_WWDT_BASE_H_
#define PLATFORM_NXP_WWDT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <irq.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const WwdtBase;
/*----------------------------------------------------------------------------*/
struct WwdtBase
{
  struct Entity parent;

  void (*handler)(void *);
  irq_t irq;
};
/*----------------------------------------------------------------------------*/
uint32_t wwdtGetClock(const struct WwdtBase *);
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_WWDT_BASE_H_ */
