/*
 * halm/platform/lpc/gen_1/flash.h
 * Copyright (C) 2015, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_FLASH_H_
#define HALM_PLATFORM_LPC_GEN_1_FLASH_H_
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
#endif /* HALM_PLATFORM_LPC_GEN_1_FLASH_H_ */
