/*
 * usb/cdc_acm_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_BASE_H_
#define USB_CDC_ACM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <containers/byte_queue.h>
#include <containers/queue.h>
#include <usb/cdc_acm_defs.h>
#include <usb/requests.h>
#include <usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const CdcAcmBase;
/*----------------------------------------------------------------------------*/
struct CdcAcmBaseConfig
{
  /** Mandatory: USB device. */
  void *device;

  /** Optional: product string. */
  const char *productString;
  /** Optional: serial string. */
  const char *serialString;
  /** Optional: vendor string. */
  const char *vendorString;

  /** Optional: product ID. Vendor ID should be initialized. */
  uint16_t product;
  /** Optional: vendor ID. Product ID should be initialized. */
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
struct CdcAcmBase
{
  struct Entity parent;

  const struct UsbDescriptor *descriptors;
  struct UsbDevice *device;
  struct CdcLineCoding lineCoding;

  struct UsbSetupPacket setupPacket;
  uint8_t *buffer;
  uint16_t dataLength;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_BASE_H_ */
