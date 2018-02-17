/*
 * halm/platform/stm/usb_device.h
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_USB_DEVICE_H_
#define HALM_PLATFORM_STM_USB_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const UsbDevice;

struct UsbDeviceConfig
{
  /** Mandatory: USB bidirectional D- line. */
  PinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  PinNumber dp;
  /** Mandatory: output pin used for soft connect feature. */
  PinNumber connect;
  /** Mandatory: monitors the presence of USB bus power. */
  PinNumber vbus;
  /** Mandatory: Vendor Identifier. */
  uint16_t vid;
  /** Mandatory: Product Identifier. */
  uint16_t pid;
  /** Optional: interrupt priority. */
  IrqPriority priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UsbDevice;

struct UsbEndpointConfig
{
  /** Mandatory: hardware device. */
  struct UsbDevice *parent;
  /** Mandatory: logical address of the endpoint. */
  uint8_t address;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_USB_DEVICE_H_ */
