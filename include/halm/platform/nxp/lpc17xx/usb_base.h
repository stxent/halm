/*
 * halm/platform/nxp/lpc17xx/usb_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_USB_BASE_H_
#define HALM_PLATFORM_NXP_LPC17XX_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UsbBase;
/*----------------------------------------------------------------------------*/
struct UsbBaseConfig
{
  /** Mandatory: USB bidirectional D- line. */
  pinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  pinNumber dp;
  /** Mandatory: output pin used for soft connect feature. */
  pinNumber connect;
  /** Mandatory: monitors the presence of USB bus power. */
  pinNumber vbus;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UsbBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_USB_BASE_H_ */
