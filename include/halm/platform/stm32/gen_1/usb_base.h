/*
 * halm/platform/stm32/gen_1/usb_base.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_GEN_1_USB_BASE_H_
#define HALM_PLATFORM_STM32_GEN_1_USB_BASE_H_
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
  /**
   * Optional: output pin used for soft connect feature.
   * Should be left unused on STM32F0xx, STM32F105 and STM32F107 series.
   */
  PinNumber connect;
  /**
   * Optional: monitors the presence of USB bus power.
   * Available on STM32F105 and STM32F107 series only.
   */
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

  /* Pin used for the soft connect feature */
  struct Pin connect;
  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void usbSoftConnectionControl(struct UsbBase *, bool);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_1_USB_BASE_H_ */
