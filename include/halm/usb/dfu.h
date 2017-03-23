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
/*----------------------------------------------------------------------------*/
struct DfuConfig
{
  /** Mandatory: USB device. */
  void *device;
  /** Optional: state transition timer. */
  void *timer;
  /** Mandatory: maximum transfer size. */
  size_t transferSize;
};
/*----------------------------------------------------------------------------*/
struct Dfu
{
  struct UsbDriver base;

  struct Timer *timer;
  struct UsbDevice *device;

  size_t (*onDownloadRequest)(size_t, const void *, size_t, uint16_t *);
  size_t (*onUploadRequest)(size_t, void *, size_t);

  /* Current offset */
  size_t position;
  /* Maximum transfer size */
  size_t transferSize;

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
void dfuOnDownloadCompleted(struct Dfu *, bool);
void dfuSetDownloadRequestCallback(struct Dfu *,
    size_t (*)(size_t, const void *, size_t, uint16_t *));
void dfuSetUploadRequestCallback(struct Dfu *,
    size_t (*)(size_t, void *, size_t));
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_DFU_H_ */
