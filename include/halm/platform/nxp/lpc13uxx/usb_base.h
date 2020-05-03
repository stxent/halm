/*
 * halm/platform/nxp/lpc13uxx/usb_base.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13UXX_USB_BASE_H_
#define HALM_PLATFORM_NXP_LPC13UXX_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/entity.h>
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UsbBase;

struct UsbBaseConfig
{
  /** Mandatory: USB bidirectional D- line. */
  PinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  PinNumber dp;
  /** Mandatory: output pin used for soft connect feature. */
  PinNumber connect;
  /** Mandatory: monitors the presence of USB bus power. */
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

  /* Pointer to an aligned array of Endpoint Command/Status entries */
  volatile uint32_t *epList;

  /* Unique peripheral identifier */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13UXX_USB_BASE_H_ */
