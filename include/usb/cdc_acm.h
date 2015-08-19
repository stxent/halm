/*
 * usb/cdc_acm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_H_
#define USB_CDC_ACM_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <containers/byte_queue.h>
#include <containers/queue.h>
#include <usb/cdc_acm_defs.h>
#include <usb/requests.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const CdcAcm;
/*----------------------------------------------------------------------------*/
struct CdcAcmConfig
{
  /** Mandatory: USB device. */
  void *device;
};
/*----------------------------------------------------------------------------*/
struct CdcAcm
{
  struct Entity parent;

  struct UsbDevice *device;

  struct UsbEndpoint *inputDataEp;
  struct UsbEndpoint *outputDataEp;
  struct UsbEndpoint *intEp;

  struct Queue outputDataReqs;

  struct ByteQueue rxfifo;
  struct CdcLineCoding lineCoding;

  struct UsbSetupPacket setupPacket;
  uint8_t *buffer;
  uint16_t dataLength;
  struct UsbRequest *request;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_H_ */
