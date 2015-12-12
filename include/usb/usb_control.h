/*
 * usb/usb_control.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_USB_CONTROL_H_
#define USB_USB_CONTROL_H_
/*----------------------------------------------------------------------------*/
#include <pin.h>
#include <containers/list.h>
#include <containers/queue.h>
#include <usb/composite_device.h>
#include <usb/usb.h>
#include <usb/usb_requests.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UsbControl;
/*----------------------------------------------------------------------------*/
struct UsbControlConfig
{
  /** Mandatory: parent device. */
  struct UsbDevice *parent;
};
/*----------------------------------------------------------------------------*/
struct UsbControl
{
  struct Entity parent;

  /* Parent object */
  struct UsbDevice *owner;

  /* List of registered descriptors */
  struct List descriptors;
  /* Device driver that is currently active */
  struct UsbDriver *driver;

  /* Control endpoints */
  struct UsbEndpoint *ep0in;
  struct UsbEndpoint *ep0out;

  /* Queue containing IN requests */
  struct Queue requestPool;

  /* Project-specific data */
  void *local;
};
/*----------------------------------------------------------------------------*/
enum result usbControlAppendDescriptor(struct UsbControl *, const void *);
uint8_t usbControlCompositeIndex(const struct UsbControl *);
void usbControlEraseDescriptor(struct UsbControl *, const void *);
enum result usbControlBindDriver(struct UsbControl *, void *);
void usbControlResetDriver(struct UsbControl *);
void usbControlUnbindDriver(struct UsbControl *, const void *);
void usbControlUpdateStatus(struct UsbControl *, uint8_t);
/*----------------------------------------------------------------------------*/
#endif /* USB_USB_CONTROL_H_ */
