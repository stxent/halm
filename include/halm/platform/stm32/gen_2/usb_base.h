/*
 * halm/platform/stm32/gen_2/usb_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_2_USB_BASE_H_
#define HALM_PLATFORM_STM32_GEN_2_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UsbBase;

struct UsbBaseConfig
{
  /** Mandatory: USB bidirectional D- line. */
  PinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  PinNumber dp;
  /** Optional: monitors the presence of USB bus power. */
  PinNumber vbus;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct UsbBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Capacity of the dedicated RAM */
  uint16_t memoryCapacity;
  /* Unique peripheral identifier */
  uint8_t channel;
  /* Number of logical endpoints */
  uint8_t numberOfEndpoints;
  /* DMA support */
  bool dma;
  /* HS support */
  bool hs;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_USB_BASE_H_ */
