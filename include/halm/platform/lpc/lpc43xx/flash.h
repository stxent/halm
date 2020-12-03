/*
 * halm/platform/lpc/lpc43xx/flash.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_FLASH_H_
#define HALM_PLATFORM_LPC_LPC43XX_FLASH_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/flash_base.h>
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Flash;

struct Flash
{
  struct FlashBase base;

  /* Current address */
  uint32_t position;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_FLASH_H_ */
