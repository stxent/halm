/*
 * halm/usb/uac.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_UAC_H_
#define HALM_USB_UAC_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
enum UacParameter
{
  /**
   * Retrieve extended information about current interface status.
   * Interface status is a bitmask, possible values are defined in the
   * \a enum \a UacFlags. Parameter type is \a uint8_t.
   */
  IF_UAC_STATUS = IF_PARAMETER_END,
  /**
   * Update information about rate feedback. Integer format is Q16.16,
   * parameter type is \a uint32_t.
   */
  IF_UAC_FEEDBACK
};

enum UacFlags
{
  UAC_SOF       = 0x01,
  UAC_SUSPENDED = 0x02,
  UAC_RX_ACTIVE = 0x04,
  UAC_RX_READY  = 0x08,
  UAC_TX_ACTIVE = 0x10,
  UAC_TX_EMPTY  = 0x20,
  UAC_RATE      = 0x40
};
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Uac;

struct Uac;

struct UacConfig
{
  /** Mandatory: USB device. */
  void *device;

  /**
   * Optional: memory region for receive and transmit buffers.
   * When the pointer is left uninitialized, buffers will be allocated in
   * the heap. Pointer address should be aligned.
   */
  void *arena;
  /** Mandatory: number of receive buffers. */
  size_t rxBuffers;
  /** Mandatory: number of transmit buffers. */
  size_t txBuffers;

  struct
  {
    /** Mandatory: identifier of the feedback endpoint. */
    uint8_t fb;
    /** Mandatory: identifier of the input data endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output data endpoint. */
    uint8_t tx;
  } endpoints;

  /** Mandatory: sample rate list, last element should be 0. */
  const uint32_t *rates;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void uacOnEvent(struct Uac *, unsigned int);
uint32_t uacOnSampleRateGet(const struct Uac *, size_t);
uint32_t uacOnSampleRateGetCurrent(const struct Uac *);
bool uacOnSampleRateSet(struct Uac *, uint32_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_UAC_H_ */
