/*
 * usb/hid_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_HID_BASE_H_
#define HALM_USB_HID_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const HidBase;
/*----------------------------------------------------------------------------*/
struct Hid;
struct HidDescriptor;
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
  } endpoint;
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
  uint8_t interfaceIndex;

  void *privateData;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_HID_BASE_H_ */
