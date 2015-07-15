/*
 * usb/usb.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_USB_H_
#define USB_USB_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <bits.h>
#include <entity.h>
/*----------------------------------------------------------------------------*/
#define EP_DIRECTION_IN                 BIT(7)
#define EP_ADDRESS(value)               (value)
#define EP_LOGICAL_ADDRESS(value)       ((value) & 0x7F)
/*----------------------------------------------------------------------------*/
enum usbDeviceStatus
{
  DEVICE_STATUS_CONNECT = 0x01,
  DEVICE_STATUS_SUSPEND = 0x02,
  DEVICE_STATUS_RESET   = 0x04
};

enum usbRequestStatus
{
  REQUEST_COMPLETED,  /* Request completed successfully */
  REQUEST_SETUP,      /* Request payload is a setup packet */
  REQUEST_STALLED,    /* Endpoint is stalled */
  REQUEST_ERROR,      /* Request is not completed */
  REQUEST_FAILED      /* Request is removed from the queue */
};
/*----------------------------------------------------------------------------*/
struct UsbRequest
{
  uint8_t *buffer;
  uint16_t capacity;
  uint16_t length;
  enum usbRequestStatus status;

  void (*callback)(struct UsbRequest *, void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
enum result usbRequestInit(struct UsbRequest *, uint16_t);
void usbRequestDeinit(struct UsbRequest *);
void usbRequestCallback(struct UsbRequest *,
    void (*)(struct UsbRequest *, void *), void *);
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbDeviceClass
{
  CLASS_HEADER

  void *(*allocate)(void *, uint16_t, uint8_t);
  enum result (*bind)(void *, void *);
  void (*setAddress)(void *, uint8_t);
  void (*setConfigured)(void *, bool);
  void (*setConnected)(void *, bool);
};
/*----------------------------------------------------------------------------*/
static inline void *usbDevAllocate(void *dev, uint16_t size, uint8_t address)
{
  return ((const struct UsbDeviceClass *)CLASS(dev))->allocate(dev, size,
      address);
}
/*----------------------------------------------------------------------------*/
static inline enum result usbDevBind(void *dev, void *driver)
{
  return ((const struct UsbDeviceClass *)CLASS(dev))->bind(dev, driver);
}
/*----------------------------------------------------------------------------*/
static inline void usbDevSetAddress(void *dev, uint8_t address)
{
  ((const struct UsbDeviceClass *)CLASS(dev))->setAddress(dev, address);
}
/*----------------------------------------------------------------------------*/
static inline void usbDevSetConfigured(void *dev, bool state)
{
  ((const struct UsbDeviceClass *)CLASS(dev))->setConfigured(dev, state);
}
/*----------------------------------------------------------------------------*/
static inline void usbDevSetConnected(void *dev, bool state)
{
  ((const struct UsbDeviceClass *)CLASS(dev))->setConnected(dev, state);
}
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbEndpointClass
{
  CLASS_HEADER

  void (*clear)(void *);
  enum result (*enqueue)(void *, struct UsbRequest *);
  bool (*isStalled)(void *);
  void (*setEnabled)(void *, bool);
  void (*setStalled)(void *, bool);
};
/*----------------------------------------------------------------------------*/
static inline void usbEpClear(void *ep)
{
  ((const struct UsbEndpointClass *)CLASS(ep))->clear(ep);
}
/*----------------------------------------------------------------------------*/
static inline enum result usbEpEnqueue(void *ep, struct UsbRequest *request)
{
  return ((const struct UsbEndpointClass *)CLASS(ep))->enqueue(ep, request);
}
/*----------------------------------------------------------------------------*/
static inline bool usbEpIsStalled(void *ep)
{
  return ((const struct UsbEndpointClass *)CLASS(ep))->isStalled(ep);
}
/*----------------------------------------------------------------------------*/
static inline void usbEpSetEnabled(void *ep, bool state)
{
  ((const struct UsbEndpointClass *)CLASS(ep))->setEnabled(ep, state);
}
/*----------------------------------------------------------------------------*/
static inline void usbEpSetStalled(void *ep, bool state)
{
  ((const struct UsbEndpointClass *)CLASS(ep))->setStalled(ep, state);
}
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbDriverClass
{
  CLASS_HEADER

  enum result (*configure)(void *, const struct UsbRequest *, uint8_t *,
      uint16_t *);
  void (*disconnect)(void *);
  const struct UsbDescriptor *(*getDescriptor)(void *);
  void (*setSuspended)(void *, bool);
};
/*----------------------------------------------------------------------------*/
static inline enum result usbDriverConfigure(void *driver,
    const struct UsbRequest *request, uint8_t *reply, uint16_t *length)
{
  return ((const struct UsbDriverClass *)CLASS(driver))->configure(driver,
      request, reply, length);
}
/*----------------------------------------------------------------------------*/
static inline void usbDriverDisconnect(void *driver)
{
  ((const struct UsbDriverClass *)CLASS(driver))->disconnect(driver);
}
/*----------------------------------------------------------------------------*/
static inline const struct UsbDescriptor *usbDriverGetDescriptor(void *driver)
{
  return ((const struct UsbDriverClass *)CLASS(driver))->getDescriptor(driver);
}
/*----------------------------------------------------------------------------*/
static inline void usbDriverSetSuspended(void *driver, bool state)
{
  ((const struct UsbDriverClass *)CLASS(driver))->setSuspended(driver, state);
}
/*----------------------------------------------------------------------------*/
#endif /* USB_USB_H_ */
