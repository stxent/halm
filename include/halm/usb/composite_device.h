/*
 * halm/usb/composite_device.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_COMPOSITE_DEVICE_H_
#define HALM_USB_COMPOSITE_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/list.h>
#include <halm/usb/usb.h>
#include <halm/usb/usb_defs.h>
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct CompositeDeviceClass
{
  struct UsbDeviceClass base;

  uint8_t (*index)(const void *);
};
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const CompositeDeviceProxy;
extern const struct CompositeDeviceClass * const CompositeDevice;
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
  struct UsbDriver base;

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
  struct Entity base;

  struct UsbDevice *device;
  struct CompositeDeviceProxy *driver;

  struct List entries;
  struct UsbConfigurationDescriptor configDescriptor;
};
/*----------------------------------------------------------------------------*/
/**
 * Calculate an index for a new interface descriptor.
 * @param device Pointer to a CompositeDevice object.
 * @return Index for an interface descriptor.
 */
static inline uint8_t usbCompositeDevIndex(const void *device)
{
  return ((const struct CompositeDeviceClass *)CLASS(device))->index(device);
}
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_COMPOSITE_DEVICE_H_ */
