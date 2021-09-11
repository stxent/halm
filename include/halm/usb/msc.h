/*
 * halm/usb/msc.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_MSC_H_
#define HALM_USB_MSC_H_
/*----------------------------------------------------------------------------*/
#include <halm/usb/usb.h>
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
extern const struct UsbDriverClass * const Msc;

struct MscQueryHandler;

struct MscConfig
{
  /** Mandatory: USB device. */
  void *device;

  /**
   * Optional: memory region for the temporary buffer. When the pointer is
   * left uninitialized, a memory for the buffer will be allocated in the heap.
   * Pointer address should be aligned.
   */
  void *arena;
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

struct Msc
{
  struct UsbDriver base;

  void (*callback)(void *);
  void *callbackArgument;

  /*
   * Buffer for temporary data. It stores commands, responses,
   * data received from the host and data to be sent to the host.
   */
  void *buffer;
  /* Size of the buffer */
  size_t bufferSize;

  struct
  {
    /* Memory interface */
    struct Interface *interface;
    /* Number of blocks */
    uint32_t blocks;
    /* Additional Sense Code */
    uint16_t asc;
    /* Sense Key */
    uint8_t sense;
    /* Status flags */
    uint8_t flags;
  } lun[1];

  /* USB device handle */
  struct UsbDevice *device;

  /* OUT endpoint handle */
  struct UsbEndpoint *rxEp;
  /* IN endpoint handle */
  struct UsbEndpoint *txEp;

  /* Addresses of bulk endpoints */
  struct
  {
    uint8_t rx;
    uint8_t tx;
  } endpoints;

  /* Size of the block in a Block Device */
  uint16_t blockSize;
  /*
   * Size of the USB packet. Packet size is initialized during interface
   * configuration and depends on the speed of the interface.
   */
  uint16_t packetSize;
  /* Interface index in configurations with multiple interface */
  uint8_t interfaceIndex;
  /* Buffer should be released during deinitialization */
  bool preallocated;

  struct
  {
    /* Pending command */
    struct
    {
      uint32_t length;
      uint32_t tag;
      uint8_t cb[16];
      uint8_t flags;
      uint8_t lun;
    } cbw;

    /* Current position in the storage */
    uint64_t position;
    /* Bytes left */
    uint32_t left;

    /* Current state of the FSM */
    uint8_t state;
  } context;

  struct MscQueryHandler *datapath;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result mscAttachUnit(struct Msc *, uint8_t, void *);
void mscDetachUnit(struct Msc *, uint8_t);
bool mscIsUnitLocked(const struct Msc *, uint8_t);
void mscSetCallback(struct Msc *, void (*)(void *), void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_H_ */
