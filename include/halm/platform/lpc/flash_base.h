/*
 * halm/platform/lpc/flash_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_LPC_FLASH_BASE_H_
#define HALM_PLATFORM_LPC_FLASH_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const FlashBase;

struct FlashBase
{
  struct Entity base;

  /* Total flash size */
  uint32_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_FLASH_BASE_H_ */
