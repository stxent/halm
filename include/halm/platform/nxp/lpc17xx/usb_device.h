/*
 * halm/platform/nxp/lpc17xx/usb_device.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_USB_DEVICE_H_
#define HALM_PLATFORM_NXP_LPC17XX_USB_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
#include <halm/usb/usb_control.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/usb_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
struct UsbEndpoint
{
  struct Entity base;

  /* Parent device */
  struct UsbDevice *device;
  /* Queued requests */
  struct Queue requests;
  /* Logical address */
  uint8_t address;
};
/*----------------------------------------------------------------------------*/
struct UsbDevice
{
  struct UsbBase base;

  /* Array of registered endpoints */
  struct UsbEndpoint *endpoints[32];
  /* Control message handler */
  struct UsbControl *control;

  /* Maximum power consumption of the device in mA */
  uint16_t power;
  /* Vendor and Product identifiers */
  uint16_t vid, pid;

  /* Device status */
  uint8_t status;
  /* Device is configured */
  bool configured;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_USB_DEVICE_H_ */
