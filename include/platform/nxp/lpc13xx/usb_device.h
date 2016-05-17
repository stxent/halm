/*
 * platform/nxp/lpc13xx/usb_device.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC13XX_USB_DEVICE_H_
#define HALM_PLATFORM_NXP_LPC13XX_USB_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <usb/usb.h>
#include <usb/usb_control.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/usb_base.h>
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
  struct UsbEndpoint *endpoints[10];
  /* Control message handler */
  struct UsbControl *control;
  /* Active device configuration */
  uint8_t configuration;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC13XX_USB_DEVICE_H_ */
