/*
 * halm/platform/stm32/flash.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FLASH_H_
#define HALM_PLATFORM_STM32_FLASH_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/flash_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Flash;

struct FlashConfig
{
  /** Optional: flash bank number. */
  enum FlashBank bank;
  /** Optional: voltage range. */
  enum VoltageRange voltage;
};

struct Flash
{
  struct FlashBase base;

  /* Current address */
  uint32_t position;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_FLASH_H_ */
