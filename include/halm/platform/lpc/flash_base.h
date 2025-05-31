/*
 * halm/platform/lpc/flash_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_FLASH_BASE_H_
#define HALM_PLATFORM_LPC_FLASH_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const FlashBase;

struct FlashGeometry;

struct FlashBaseConfig
{
  /** Optional: flash bank. */
  uint8_t bank;
};

struct FlashBase
{
  struct Entity base;

  /* Total flash size */
  uint32_t size;
  /* Flash bank */
  uint8_t bank;
  /* Flash has uniform sector size */
  bool uniform;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

size_t flashBaseGetGeometry(const struct FlashBase *, struct FlashGeometry *,
    size_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_FLASH_BASE_H_ */
