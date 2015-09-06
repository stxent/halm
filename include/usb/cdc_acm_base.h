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
  /** Optional: event callback function. */
  void (*callback)(void *);
  /** Optional: callback function argument. */
  void *argument;

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
  struct UsbDriver parent;

  void (*callback)(void *);
  void *callbackArgument;

  struct UsbDevice *device;

  const struct UsbDescriptor **descriptorArray;
  struct UsbEndpointDescriptor *endpointDescriptors;
  struct UsbDescriptor *stringDescriptor;

  bool suspended;

  struct
  {
    struct CdcLineCoding coding;
    bool dtr;
    bool rts;
    bool updated;
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
