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
#include <entity.h>
/*----------------------------------------------------------------------------*/
enum usbEpStatus
{
  EP_STATUS_DATA    = 0x01, /* EP has data */
  EP_STATUS_STALLED = 0x02, /* EP is stalled */
  EP_STATUS_SETUP   = 0x04, /* EP received setup packet */
  EP_STATUS_ERROR   = 0x08, /* EP data was overwritten by setup packet */
  EP_STATUS_NACKED  = 0x10  /* EP sent NAK */
};

//enum usbDevStatus
//{
//  DEV_STATUS_CONNECT = 0x01,
//  DEV_STATUS_SUSPEND = 0x02,
//  DEV_STATUS_RESET   = 0x04
//};

//enum usbIntStatus
//{
//  INACK_CI = 0x01, /* Interrupt on NACK for control in */
//  INACK_CO = 0x02, /* Interrupt on NACK for control out */
//  INACK_II = 0x04, /* Interrupt on NACK for interrupt in */
//  INACK_IO = 0x08, /* Interrupt on NACK for interrupt out */
//  INACK_BI = 0x10, /* Interrupt on NACK for bulk in */
//  INACK_BO = 0x20  /* Interrupt on NACK for bulk out */
//};
/*----------------------------------------------------------------------------*/
struct UsbRequest
{
  uint8_t *buffer;
  uint16_t capacity;
  uint16_t length;
  uint8_t status;

  void (*callback)(struct UsbRequest *, void *);
  void *callbackArgument;
};
/*----------------------------------------------------------------------------*/
enum result usbRequestInit(struct UsbRequest *, uint16_t);
void usbRequestDeinit(struct UsbRequest *);
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbDeviceClass
{
  CLASS_HEADER

  void *(*allocate)(void *, uint16_t, uint8_t);
};
/*----------------------------------------------------------------------------*/
static inline void *usbDevAllocate(void *device, uint16_t size, uint8_t address)
{
  return ((const struct UsbDeviceClass *)CLASS(device))->allocate(device, size,
      address);
}
/*----------------------------------------------------------------------------*/
/* Class descriptor */
struct UsbEndpointClass
{
  CLASS_HEADER

  enum result (*enqueue)(void *, struct UsbRequest *);
  void (*erase)(void *, const struct UsbRequest *);
  void (*setEnabled)(void *, bool);
  void (*setStalled)(void *, bool);
};
/*----------------------------------------------------------------------------*/
static inline enum result usbEpEnqueue(void *endpoint,
    struct UsbRequest *request)
{
  return ((const struct UsbEndpointClass *)CLASS(endpoint))->enqueue(endpoint,
      request);
}
/*----------------------------------------------------------------------------*/
static inline void usbEpErase(void *endpoint, const struct UsbRequest *request)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->erase(endpoint, request);
}
/*----------------------------------------------------------------------------*/
static inline void usbEpSetEnabled(void *endpoint, bool state)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->setEnabled(endpoint,
      state);
}
/*----------------------------------------------------------------------------*/
static inline void usbEpSetStalled(void *endpoint, bool state)
{
  ((const struct UsbEndpointClass *)CLASS(endpoint))->setStalled(endpoint,
      state);
}
/*----------------------------------------------------------------------------*/
#endif /* USB_USB_H_ */
