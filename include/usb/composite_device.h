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
extern const struct UsbDriverClass * const CompositeDevice;
/*----------------------------------------------------------------------------*/
struct UsbControl;
/*----------------------------------------------------------------------------*/
struct CompositeDeviceConfig
{
  /** Mandatory: USB control object. */
  void *control;
};
/*----------------------------------------------------------------------------*/
struct CompositeDevice
{
  struct UsbDriver parent;

  struct UsbControl *control;

  struct List entries;
  struct UsbConfigurationDescriptor configDescriptor;
};
/*----------------------------------------------------------------------------*/
enum result compositeDeviceAttach(struct CompositeDevice *, void *);
void compositeDeviceDetach(struct CompositeDevice *, const void *);
void compositeDeviceUpdate(struct CompositeDevice *, uint8_t, uint16_t);
/*----------------------------------------------------------------------------*/
#endif /* USB_COMPOSITE_DEVICE_H_ */
