/*
 * usb/msc.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef USB_MSC_H_
#define USB_MSC_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
#include <containers/queue.h>
#include <usb/usb.h>
#include <usb/usb_defs.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const Msc;
/*----------------------------------------------------------------------------*/
struct MscConfig
{
  /** Mandatory: USB device. */
  void *device;

  /** Mandatory: storage interface. */
  struct Interface *storage;

  struct
  {
    /** Mandatory: identifier of the input endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output endpoint. */
    uint8_t tx;
  } endpoint;
};
/*----------------------------------------------------------------------------*/
struct Msc
{
  struct UsbDriver base;

  struct UsbDevice *device;
  struct Interface *storage;

  struct Queue rxRequestQueue;
  struct Queue txRequestQueue;

  struct UsbEndpoint *rxEp;
  struct UsbEndpoint *txEp;

//  /* Buffer between USB and memory interface */
//  uint8_t *storageBuffer;

  uint8_t stage;

  uint64_t position; /* R/W offset */
  uint32_t length; /* R/W length */
  size_t chunk; //FIXME

  void *privateData;
};
/*----------------------------------------------------------------------------*/
#endif /* USB_MSC_H_ */
