/*
 * halm/usb/cdc_acm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_CDC_ACM_H_
#define HALM_USB_CDC_ACM_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/generic/serial.h>
#include <halm/usb/cdc_acm_base.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum CdcAcmParameter
{
  /** Retrieve extended information about current interface status. */
  IF_CDC_ACM_STATUS = IF_SERIAL_PARAMETER_END
};

enum
{
  CDC_ACM_RX_AVAILABLE = 0x01,
  CDC_ACM_TX_EMPTY     = 0x02,
  CDC_ACM_SUSPENDED    = 0x04,
  CDC_ACM_LINE_CHANGED = 0x08
};
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const CdcAcm;

struct CdcAcmConfig
{
  /** Mandatory: USB device. */
  void *device;

  /**
   * Mandatory: number of reception buffers. All types of buffers have
   * fixed size of 64 bytes.
   */
  size_t rxBuffers;
  /** Mandatory: number of transmission buffers. */
  size_t txBuffers;

  struct
  {
    /** Mandatory: identifier of the notification endpoint. */
    uint8_t interrupt;
    /** Mandatory: identifier of the input data endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output data endpoint. */
    uint8_t tx;
  } endpoints;
};

struct CdcAcm
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Lower half of the driver */
  struct CdcAcmBase *driver;
  /* Queue for OUT requests */
  PointerQueue rxRequestQueue;
  /* Pool for IN requests */
  PointerArray txRequestPool;
  /* Pointer to the beginning of the request pool */
  void *requests;
  /* Number of available and pending bytes */
  size_t queuedRxBytes, queuedTxBytes;

  struct UsbEndpoint *rxDataEp;
  struct UsbEndpoint *txDataEp;
  struct UsbEndpoint *notificationEp;

  bool suspended;
  bool updated;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void cdcAcmOnParametersChanged(struct CdcAcm *);
void cdcAcmOnEvent(struct CdcAcm *, unsigned int);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_CDC_ACM_H_ */
