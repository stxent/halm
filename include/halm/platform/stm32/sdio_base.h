/*
 * halm/platform/stm32/sdio_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_SDIO_BASE_H_
#define HALM_PLATFORM_STM32_SDIO_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/platform/stm32/dma.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SdioBase;

struct SdioBaseConfig
{
  /** Mandatory: clock line. */
  PinNumber clk;
  /** Mandatory: command line. */
  PinNumber cmd;
  /** Mandatory: data line 0. */
  PinNumber dat0;
  /** Optional: data line 1. */
  PinNumber dat1;
  /** Optional: data line 2. */
  PinNumber dat2;
  /** Optional: data line 3. */
  PinNumber dat3;
};

struct SdioBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Bus width */
  uint8_t width;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void sdioConfigPins(struct SdioBase *, const struct SdioBaseConfig *);

/* Platform-specific functions */
void *sdioMakeOneShotDma(uint8_t, enum DmaPriority, enum DmaType);
uint32_t sdioGetClock(const struct SdioBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_SDIO_BASE_H_ */
