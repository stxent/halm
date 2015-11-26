/*
 * usb/cdc_acm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_H_
#define USB_CDC_ACM_H_
/*----------------------------------------------------------------------------*/
#include <containers/byte_queue.h>
#include <containers/queue.h>
#include <interface.h>
#include <usb/cdc_acm_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const CdcAcm;
/*----------------------------------------------------------------------------*/
enum cdcAcmOption
{
  /** Retrieve extended information about current interface status. */
  IF_CDC_ACM_STATUS = IF_OPTION_END
};
/*----------------------------------------------------------------------------*/
enum
{
  CDC_ACM_RX_AVAILABLE = 0x01,
  CDC_ACM_TX_EMPTY     = 0x02,
  CDC_ACM_SUSPENDED    = 0x04,
  CDC_ACM_LINE_CHANGED = 0x08
};
/*----------------------------------------------------------------------------*/
struct CdcAcmConfig
{
  /** Mandatory: USB device. */
  void *device;

  /** Optional: input queue size. */
  uint32_t rxLength;
  /** Optional: output queue size. */
  uint32_t txLength;

  /** Optional: serial number string. */
  const char *serial;

  struct
  {
    /** Mandatory: identifier of the notification endpoint. */
    uint8_t interrupt;
    /** Mandatory: identifier of the input data endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output data endpoint. */
    uint8_t tx;
  } endpoint;

  /** Optional: enable composite device mode. */
  bool composite;
};
/*----------------------------------------------------------------------------*/
struct CdcAcm
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  /* Lower half of the driver */
  struct CdcAcmBase *driver;
  /* Input and output queues */
  struct ByteQueue rxQueue, txQueue;
  /* Request queues */
  struct Queue rxRequestQueue, txRequestQueue;
  /* Pointer to the beginning of the request pool */
  struct UsbRequest *requests;
  /* Size of the queued data */
  uint16_t queuedRxBytes;

  struct UsbEndpoint *rxDataEp;
  struct UsbEndpoint *txDataEp;
  struct UsbEndpoint *notificationEp;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_H_ */
