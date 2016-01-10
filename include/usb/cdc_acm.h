/*
 * usb/cdc_acm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_H_
#define USB_CDC_ACM_H_
/*----------------------------------------------------------------------------*/
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

  /**
   * Mandatory: number of reception buffers. All types of buffers have
   * fixed size of 64 bytes.
   */
  uint8_t rxBuffers;
  /** Mandatory: number of transmission buffers. */
  uint8_t txBuffers;

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
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Lower half of the driver */
  struct CdcAcmBase *driver;
  /* Request queues */
  struct Queue rxRequestQueue, txRequestQueue;
  /* Pointer to the beginning of the request pool */
  void *requests;
  /* Sizes of queued data */
  uint16_t queuedRxBytes, queuedTxBytes;

  struct UsbEndpoint *rxDataEp;
  struct UsbEndpoint *txDataEp;
  struct UsbEndpoint *notificationEp;

  bool suspended;
  bool updated;
  bool zeroPacketRequired;
};
/*----------------------------------------------------------------------------*/
void cdcAcmOnParametersChanged(struct CdcAcm *);
void cdcAcmOnStatusChanged(struct CdcAcm *, uint8_t);
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_H_ */
