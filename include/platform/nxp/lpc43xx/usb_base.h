/*
 * platform/nxp/lpc43xx/usb_base.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC43XX_USB_BASE_H_
#define PLATFORM_NXP_LPC43XX_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <entity.h>
#include <irq.h>
#include <pin.h>
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
#endif /* PLATFORM_NXP_LPC43XX_USB_BASE_H_ */
