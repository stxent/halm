/*
 * halm/platform/stm32/dcmi_base.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DCMI_BASE_H_
#define HALM_PLATFORM_STM32_DCMI_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/platform/stm32/dma.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const DcmiBase;

struct DcmiBaseConfig
{
  /** Camera interface pins. */
  struct
  {
    /** Mandatory: array of data lines terminated by a zero element. */
    const PinNumber *data;
    /** Optional: horizontal synchronization line. */
    PinNumber hsync;
    /** Optional: vertical synchronization line. */
    PinNumber vsync;
    /** Mandatory: pixel clock line. */
    PinNumber pixclk;
  } pins;
};

struct DcmiBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Platform-specific functions */
void *dcmiMakeListDma(size_t, uint8_t, enum DmaPriority);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DCMI_BASE_H_ */
