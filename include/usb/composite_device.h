/*
 * usb/composite_device.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_COMPOSITE_DEVICE_H_
#define USB_COMPOSITE_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <containers/list.h>
#include <usb/usb.h>
#include <usb/usb_requests.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const CompositeDeviceProxy;
extern const struct UsbDeviceClass * const CompositeDevice;
/*----------------------------------------------------------------------------*/
struct CompositeDevice;
/*----------------------------------------------------------------------------*/
struct CompositeDeviceProxyConfig
{
  /** Mandatory: Composite Device object. */
  struct CompositeDevice *owner;
};
/*----------------------------------------------------------------------------*/
struct CompositeDeviceProxy
{
  struct UsbDriver parent;

  struct CompositeDevice *owner;
};
/*----------------------------------------------------------------------------*/
struct CompositeDeviceConfig
{
  /** Mandatory: USB device. */
  void *device;
};
/*----------------------------------------------------------------------------*/
struct CompositeDevice
{
  struct Entity parent;

  struct UsbDevice *device;
  struct CompositeDeviceProxy *driver;

  struct List entries;
  struct UsbConfigurationDescriptor configDescriptor;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_COMPOSITE_DEVICE_H_ */
