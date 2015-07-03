/*
 * platform/nxp/gen_1/usb_device_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_GEN_1_USB_DEVICE_BASE_H_
#define PLATFORM_NXP_GEN_1_USB_DEVICE_BASE_H_
/*----------------------------------------------------------------------------*/
#include <spinlock.h>
#include <containers/list.h>
#include <containers/queue.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/PLATFORM/usb_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct UsbDeviceClass * const UsbDeviceBase;
extern const struct UsbEndpointClass * const UsbEndpoint;
/*----------------------------------------------------------------------------*/
struct UsbDeviceBaseConfig
{
  /** Mandatory: USB bidirectional D- line. */
  pin_t dm;
  /** Mandatory: USB bidirectional D+ line. */
  pin_t dp;
  /** Mandatory: output pin used for soft connect feature. */
  pin_t connect;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct UsbDeviceBase
{
  struct UsbBase parent;

  /* List of registered endpoints */
  struct List endpoints;
  /* Protects the list of endpoints */
  spinlock_t spinlock;
};
/*----------------------------------------------------------------------------*/
struct UsbEndpointConfig
{
  /** Mandatory: hardware device. */
  struct UsbDeviceBase *parent;
  /** Mandatory: maximum packet size. */
  uint16_t size;
  /** Mandatory: logical address of the endpoint. */
  uint8_t address;
};
/*----------------------------------------------------------------------------*/
struct UsbEndpoint
{
  struct Entity parent;

  /* Parent device */
  struct UsbDeviceBase *device;
  /* Pending requests */
  struct Queue requests;
  /* Maximum packet size */
  uint16_t size;
  /* Logical address */
  uint8_t address;
  /* Busy flag */
  bool busy;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_GEN_1_USB_DEVICE_BASE_H_ */
