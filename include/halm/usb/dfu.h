/*
 * halm/usb/dfu.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_DFU_H_
#define HALM_USB_DFU_H_
/*----------------------------------------------------------------------------*/
#include <halm/timer.h>
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const Dfu;

struct DfuConfig
{
  /** Mandatory: USB device. */
  void *device;
  /** Optional: state transition timer. */
  void *timer;
  /** Mandatory: maximum transfer size. */
  uint16_t transferSize;
};

struct Dfu
{
  struct UsbDriver base;

  struct Timer *timer;
  struct UsbDevice *device;

  void *callbackArgument;
  void (*onDetachRequest)(void *, uint16_t);
  size_t (*onDownloadRequest)(void *, size_t, const void *, size_t, uint16_t *);
  size_t (*onUploadRequest)(void *, size_t, void *, size_t);

  /* Current offset */
  size_t position;
  /* Maximum transfer size */
  uint16_t transferSize;

  /* Timeout for the current operation */
  uint16_t timeout;

  /* Interface index in configurations with multiple interfaces */
  uint8_t interfaceIndex;
  /* Current device state */
  uint8_t state;
  /* Status of the last operation */
  uint8_t status;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void dfuOnDownloadCompleted(struct Dfu *, bool);
void dfuSetCallbackArgument(struct Dfu *, void *);
void dfuSetDetachRequestCallback(struct Dfu *, void (*)(void *, uint16_t));
void dfuSetDownloadRequestCallback(struct Dfu *,
    size_t (*)(void *, size_t, const void *, size_t, uint16_t *));
void dfuSetUploadRequestCallback(struct Dfu *,
    size_t (*)(void *, size_t, void *, size_t));

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_DFU_H_ */
