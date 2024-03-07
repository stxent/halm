/*
 * halm/platform/numicro/m48x/flash.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_M48X_FLASH_H_
#define HALM_PLATFORM_NUMICRO_M48X_FLASH_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Flash;

enum [[gnu::packed]] FlashBank
{
  FLASH_APROM,
  FLASH_BANK_0 = FLASH_APROM,
  FLASH_BANK_1,
  FLASH_CONFIG,
  FLASH_KPROM,
  FLASH_LDROM,
  FLASH_OTP,
  FLASH_SPROM,

  /* End of the list */
  FLASH_BANK_END
};

struct FlashConfig
{
  /** Mandatory: flash bank. */
  enum FlashBank bank;
};

struct Flash
{
  struct Interface base;

  /* Current address */
  uint32_t position;
  /* Flash size */
  uint32_t size;
  /* Flash bank */
  uint8_t bank;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_FLASH_H_ */
