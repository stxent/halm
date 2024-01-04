/*
 * halm/platform/stm32/gen_2/usb_device.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_USB_DEVICE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_2_USB_DEVICE_H_
#define HALM_PLATFORM_STM32_GEN_2_USB_DEVICE_H_
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
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_USB_DEVICE_H_ */
