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
};
/*----------------------------------------------------------------------------*/
struct CdcAcmBase
{
  struct Entity parent;

  struct UsbDevice *device;
  struct CdcLineCoding lineCoding;

  struct UsbSetupPacket setupPacket;
  uint8_t *buffer;
  uint16_t dataLength;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_BASE_H_ */
