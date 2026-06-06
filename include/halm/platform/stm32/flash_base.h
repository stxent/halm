/*
 * halm/platform/stm32/flash_base.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_FLASH_BASE_H_
#define HALM_PLATFORM_STM32_FLASH_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/stm32/system.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const FlashBase;

enum [[gnu::packed]] FlashBank
{
  FLASH_BANK_1,
  FLASH_BANK_2,

  /* End of the list */
  FLASH_BANK_END
};

struct FlashGeometry;

struct FlashBaseConfig
{
  /** Optional: flash bank number. */
  enum FlashBank bank;
  /** Optional: voltage range. */
  enum VoltageRange voltage;
};

struct FlashBase
{
  struct Entity base;

  /* Total flash size */
  uint32_t size;
  /* Flash bank */
  enum FlashBank bank;
  /* Parallelism configuration */
  uint8_t parallelism;
  /* Large sectors are used */
  bool large;
  /* All sectors have uniform size */
  bool uniform;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result flashBaseErase(struct FlashBase *, uint32_t);
uintptr_t flashBaseGetAddress(const struct FlashBase *);
size_t flashBaseGetGeometry(const struct FlashBase *, struct FlashGeometry *,
    size_t);
enum Result flashBaseWrite(struct FlashBase *, uint32_t, const void *, size_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_FLASH_BASE_H_ */
