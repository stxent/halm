/*
 * halm/usb/cdc_acm_base.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_CDC_ACM_BASE_H_
#define HALM_USB_CDC_ACM_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const CdcAcmBase;

struct CdcAcm;
struct CdcAcmBase;

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
  } endpoints;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint8_t cdcAcmBaseGetInterfaceIndex(const struct CdcAcmBase *);
uint32_t cdcAcmBaseGetRate(const struct CdcAcmBase *);
uint8_t cdcAcmBaseGetState(const struct CdcAcmBase *);
uint8_t cdcAcmBaseGetUsbSpeed(const struct CdcAcmBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_CDC_ACM_BASE_H_ */
