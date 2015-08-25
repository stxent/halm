/*
 * usb/cdc_acm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_H_
#define USB_CDC_ACM_H_
/*----------------------------------------------------------------------------*/
#include <containers/byte_queue.h>
#include <interface.h>
#include <usb/cdc_acm_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const CdcAcm;
/*----------------------------------------------------------------------------*/
struct CdcAcmConfig
{
  /** Mandatory: USB device. */
  void *device;
  /** Optional: input queue size. */
  uint32_t rxLength;
  /** Optional: output queue size. */
  uint32_t txLength;

  /** Optional: product string. */
  const char *productString;
  /** Optional: serial string. */
  const char *serialString;
  /** Optional: vendor string. */
  const char *vendorString;

  /** Optional: product ID. */
  uint16_t product;
  /** Optional: vendor ID. */
  uint16_t vendor;

  struct
  {
    /** Mandatory: identifier of the notification endpoint. */
    uint8_t interrupt;
    /** Mandatory: identifier of the input data endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output data endpoint. */
    uint8_t tx;
  } endpoint;
};
/*----------------------------------------------------------------------------*/
struct CdcAcm
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Low-level USB driver */
  struct CdcAcmBase *driver;
  /* Input and output queues */
  struct ByteQueue rxQueue, txQueue;
  /* Request queues */
  struct Queue rxRequestQueue, txRequestQueue;
  /* Size of the queued data */
  uint16_t queuedRxBytes;

  struct UsbEndpoint *rxDataEp;
  struct UsbEndpoint *txDataEp;
  struct UsbEndpoint *notificationEp;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_H_ */
