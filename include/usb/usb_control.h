/*
 * usb/usb_control.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_USB_CONTROL_H_
#define HALM_USB_USB_CONTROL_H_
/*----------------------------------------------------------------------------*/
#include <pin.h>
#include <containers/list.h>
#include <containers/queue.h>
#include <usb/usb.h>
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
  struct Entity base;

  /* Parent object */
  struct UsbDevice *owner;

  /* List of registered descriptors */
  struct List descriptors;
  /* Device driver that is currently active */
  struct UsbDriver *driver;

  /* Control endpoints */
  struct UsbEndpoint *ep0in;
  struct UsbEndpoint *ep0out;

  /* Queue for IN requests */
  struct Queue inRequestPool;
  /* Single OUT request */
  struct UsbRequest *outRequest;

  /* Project-specific data */
  void *local;
};
/*----------------------------------------------------------------------------*/
enum result usbControlAppendDescriptor(struct UsbControl *, const void *);
void usbControlEraseDescriptor(struct UsbControl *, const void *);
enum result usbControlBindDriver(struct UsbControl *, void *);
void usbControlResetDriver(struct UsbControl *);
void usbControlUnbindDriver(struct UsbControl *);
void usbControlEvent(struct UsbControl *, unsigned int);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_USB_CONTROL_H_ */
