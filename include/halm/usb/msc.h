/*
 * halm/usb/msc.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_MSC_H_
#define HALM_USB_MSC_H_
/*----------------------------------------------------------------------------*/
#include <xcore/containers/queue.h>
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const Msc;
/*----------------------------------------------------------------------------*/
struct MscConfig
{
  /** Mandatory: USB device. */
  void *device;

  /** Mandatory: storage interface. */
  struct Interface *storage;

  /**
   * Optional: pointer to a buffer for a temporary data. When the pointer is
   * left uninitialized, a memory for the buffer will be allocated in the heap.
   * Buffer should be aligned along 4-byte boundary.
   */
  void *buffer;
  /** Mandatory: buffer size. */
  size_t size;

  struct
  {
    /** Mandatory: identifier of the input endpoint. */
    uint8_t rx;
    /** Mandatory: identifier of the output endpoint. */
    uint8_t tx;
  } endpoints;
};
/*----------------------------------------------------------------------------*/
struct Msc
{
  struct UsbDriver base;

  struct UsbDevice *device;
  struct Interface *storage;

  struct Queue controlQueue;
  struct Queue rxQueue;
  struct Queue txQueue;

  struct UsbEndpoint *rxEp;
  struct UsbEndpoint *txEp;

  /*
   * Buffer for temporary data. It holds responses to control commands,
   * data received from the host or data to be sent to the host.
   */
  uint8_t *buffer;
  /* Size of the buffer */
  size_t bufferSize;

  uint32_t blockCount;
  uint32_t blockLength;

  /*
   * Size of the USB packet. Packet size is initialized during interface
   * configuration and depends on the speed of the interface.
   */
  uint16_t packetSize;

  /* Addresses of endpoints */
  struct
  {
    uint8_t rx;
    uint8_t tx;
  } endpoints;

  /* Interface index in configurations with multiple interface */
  uint8_t interfaceIndex;
  /* Current state of the FSM */
  uint8_t state;
  /* Composite device flag */
  bool composite;

  void *privateData;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_H_ */
