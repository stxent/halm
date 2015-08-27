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
struct CdcAcmBase
{
  struct Entity parent;

  const struct UsbDescriptor **descriptorArray;
  struct UsbEndpointDescriptor *endpointDescriptors;
  struct UsbDescriptor *stringDescriptor;

  struct UsbDevice *device;

  struct
  {
    struct CdcLineCoding coding;
    bool dtr;
    bool rts;
  } line;

  struct
  {
    struct UsbSetupPacket packet;
    uint8_t *buffer;
    uint16_t left;
  } state;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_BASE_H_ */
