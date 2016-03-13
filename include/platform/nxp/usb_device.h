/*
 * platform/nxp/usb_device.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_USB_DEVICE_H_
#define PLATFORM_NXP_USB_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <containers/list.h>
#include <containers/queue.h>
#include <usb/usb.h>
#include <usb/usb_control.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const UsbDevice;
extern const struct UsbEndpointClass * const UsbEndpoint;
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/usb_device.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
struct UsbDeviceConfig
{
  /** Mandatory: USB bidirectional D- line. */
  pinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  pinNumber dp;
  /** Mandatory: output pin used for soft connect feature. */
  pinNumber connect;
  /** Mandatory: monitors the presence of USB bus power. */
  pinNumber vbus;
  /** Optional: interrupt priority. */
  irqPriority priority;
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
#endif /* PLATFORM_NXP_USB_DEVICE_H_ */
