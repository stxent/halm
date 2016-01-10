/*
 * usb/cdc_acm_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_CDC_ACM_BASE_H_
#define USB_CDC_ACM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <usb/cdc_acm_defs.h>
#include <usb/usb.h>
#include <usb/usb_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const CdcAcmBase;
/*----------------------------------------------------------------------------*/
struct CdcAcm;
/*----------------------------------------------------------------------------*/
struct CdcAcmBaseConfig
{
  /** Mandatory: pointer to an upper half of the driver. */
  struct CdcAcm *owner;
  /** Mandatory: USB device. */
  void *device;

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
  struct UsbDriver base;

  struct CdcAcm *owner;
  struct UsbDevice *device;

  struct CdcLineCoding lineCoding;
  uint8_t controlLineState;
  uint8_t controlInterfaceIndex;

  void *local;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_CDC_ACM_BASE_H_ */
