/*
 * usb/usb_device.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_USB_DEVICE_H_
#define USB_USB_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <pin.h>
#include <containers/queue.h>
#include <usb/requests.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/usb_device_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const UsbDevice;
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
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UsbDevice
{
  struct Entity parent;

  struct UsbDeviceBase *base;
  struct UsbDriver *driver;

  struct UsbEndpoint *ep0in;
  struct UsbEndpoint *ep0out;
  struct Queue requestPool;
  struct UsbRequest *requests;

  uint8_t currentConfiguration;

  struct
  {
    struct UsbSetupPacket packet;
    uint8_t *buffer;
    uint16_t left;
  } state;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_USB_DEVICE_H_ */
