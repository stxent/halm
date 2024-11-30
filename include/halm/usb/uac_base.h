/*
 * halm/usb/uac_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_UAC_BASE_H_
#define HALM_USB_UAC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const UacBase;

struct Uac;
struct UacBase;

struct UacBaseConfig
{
  /** Mandatory: pointer to an upper half of the driver. */
  struct Uac *owner;
  /** Mandatory: USB device. */
  void *device;

  struct
  {
    /** Mandatory: identifier of the feedback endpoint. */
    uint8_t fb;
    /** Mandatory: identifier of the input data endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output data endpoint. */
    uint8_t tx;
  } endpoints;

  /** Mandatory: maximum packet size in bytes for 1 kHz frame rate. */
  size_t packet;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint8_t uacBaseGetInterfaceIndex(const struct UacBase *);
size_t uacBaseGetPacketSize(const struct UacBase *);
enum UsbSpeed uacBaseGetUsbSpeed(const struct UacBase *);
bool uacBaseIsRxActive(const struct UacBase *);
bool uacBaseIsTxActive(const struct UacBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_UAC_BASE_H_ */
