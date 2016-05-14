/*
 * usb/cdc_acm_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_CDC_ACM_BASE_H_
#define HALM_USB_CDC_ACM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <usb/usb.h>
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

  /* Upper-half driver */
  struct CdcAcm *owner;
  /* USB peripheral */
  struct UsbDevice *device;

  /* Interface index in configurations with multiple interface */
  uint8_t controlInterfaceIndex;
  /* Speed of the USB interface */
  uint8_t speed;

  void *privateData;
};
/*----------------------------------------------------------------------------*/
uint32_t cdcAcmBaseGetRate(const struct CdcAcmBase *);
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_CDC_ACM_BASE_H_ */
