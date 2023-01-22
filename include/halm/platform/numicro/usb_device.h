/*
 * halm/platform/numicro/usb_device.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_USB_DEVICE_H_
#define HALM_PLATFORM_NUMICRO_USB_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const UsbDevice;

struct UsbDeviceConfig
{
  /** Mandatory: USB bidirectional D- line. */
  PinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  PinNumber dp;
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
struct UsbEndpointConfig
{
  /** Mandatory: hardware device. */
  struct UsbDevice *parent;
  /** Mandatory: logical address of the endpoint. */
  uint8_t address;
  /** Mandatory: physical address of the endpoint. */
  uint8_t index;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_USB_DEVICE_H_ */
