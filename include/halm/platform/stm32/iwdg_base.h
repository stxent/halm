/*
 * halm/platform/stm32/iwdg_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_IWDG_BASE_H_
#define HALM_PLATFORM_STM32_IWDG_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const IwdgBase;

struct IwdgBase
{
  struct Entity base;

  /* Watchdog reset occurred */
  bool fired;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint32_t iwdgGetClock(const struct IwdgBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_IWDG_BASE_H_ */
