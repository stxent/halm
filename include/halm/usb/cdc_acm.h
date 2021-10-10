/*
 * halm/usb/cdc_acm.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_CDC_ACM_H_
#define HALM_USB_CDC_ACM_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/serial.h>
#include <xcore/interface.h>
#include <stdint.h>
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

struct CdcAcm;

struct CdcAcmConfig
{
  /** Mandatory: USB device. */
  void *device;

  /**
   * Optional: memory region for receive and transmit buffers.
   * When the pointer is left uninitialized, buffers will be allocated in
   * the heap. Pointer address should be aligned. Buffers have fixed size of
   * 64 bytes for full-speed devices and 512 bytes for high-speed devices.
   */
  void *arena;
  /** Mandatory: number of receive buffers. */
  size_t rxBuffers;
  /** Mandatory: number of transmit buffers. */
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
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void cdcAcmOnParametersChanged(struct CdcAcm *);
void cdcAcmOnEvent(struct CdcAcm *, unsigned int);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_CDC_ACM_H_ */
