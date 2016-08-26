/*
 * halm/usb/hid_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_HID_BASE_H_
#define HALM_USB_HID_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const HidBase;
/*----------------------------------------------------------------------------*/
struct Hid;
/*----------------------------------------------------------------------------*/
struct HidBaseConfig
{
  /** Mandatory: pointer to an upper half of the driver. */
  struct Hid *owner;
  /** Mandatory: USB device. */
  void *device;

  /** Mandatory: report descriptor. */
  const void *descriptor;
  /** Mandatory: size of the report descriptor. */
  uint16_t descriptorSize;
  /** Mandatory: size of the report. */
  uint16_t reportSize;

  struct
  {
    /** Mandatory: identifier of the notification endpoint. */
    uint8_t interrupt;
  } endpoints;

  /** Optional: enable composite device mode. */
  bool composite;
};
/*----------------------------------------------------------------------------*/
struct HidBase
{
  struct UsbDriver base;

  struct Hid *owner;
  struct UsbDevice *device;

  const void *reportDescriptor;
  uint16_t reportDescriptorSize;
  uint8_t idleTime;

  /* Address of the interrupt endpoint */
  uint8_t endpointAddress;
  /* Interface index in configurations with multiple interfaces */
  uint8_t interfaceIndex;
  /* Composite device flag */
  bool composite;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_HID_BASE_H_ */
