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
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Flash;

enum FlashBank
{
  FLASH_BANK_A,
  FLASH_BANK_B,
  FLASH_BANK_CURRENT,
  FLASH_BANK_SPARE,

  /* End of the list */
  FLASH_BANK_END
} __attribute__((packed));

struct FlashConfig
{
  /** Optional: flash bank. */
  enum FlashBank bank;
};

struct Flash
{
  struct FlashBase base;

  /* Current address */
  uint32_t position;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_FLASH_H_ */
