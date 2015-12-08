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
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/usb_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const UsbDevice;
extern const struct UsbEndpointClass * const UsbEndpoint;
/*----------------------------------------------------------------------------*/
struct UsbDeviceConfig
{
  /** Mandatory: USB bidirectional D- line. */
  pin_t dm;
  /** Mandatory: USB bidirectional D+ line. */
  pin_t dp;
  /** Mandatory: output pin used for soft connect feature. */
  pin_t connect;
  /** Mandatory: monitors the presence of USB bus power. */
  pin_t vbus;
  /** Optional: interrupt priority. */
  priority_t priority;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UsbDevice
{
  struct UsbBase parent;

  /* List of registered endpoints */
  struct List endpoints;
  /* Control message handler */
  struct UsbControl *control;
  /* Active device configuration */
  uint8_t configuration;
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
struct UsbEndpoint
{
  struct Entity parent;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  struct Queue requests;
  /* Logical address */
  uint8_t address;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_USB_DEVICE_H_ */
