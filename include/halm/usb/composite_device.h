/*
 * halm/usb/composite_device.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_COMPOSITE_DEVICE_H_
#define HALM_USB_COMPOSITE_DEVICE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_list.h>
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const CompositeDevice;

struct CompositeDeviceConfig
{
  /** Mandatory: USB device. */
  void *device;
};

struct CompositeDevice
{
  struct Entity base;

  struct UsbDevice *parent;
  struct CompositeDeviceProxy *driver;

  PointerList entries;

  /* Length of the unified configuration descriptor */
  uint16_t configurationLength;
  /* Number of interface descriptors */
  uint8_t interfaceCount;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_COMPOSITE_DEVICE_H_ */
