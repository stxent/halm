/*
 * halm/usb/msc_private.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_MSC_PRIVATE_H_
#define HALM_USB_MSC_PRIVATE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/usb/msc_defs.h>
#include <halm/usb/usb.h>
#include <halm/usb/usb_request.h>
/*----------------------------------------------------------------------------*/
struct Interface;
struct MscQueryHandler;

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

      union
      {
        uint8_t raw[16];

        struct PreventAllowMediumRemovalCommand mediumRemoval;
        struct ReadCapacity10Command readCapacity10;
        struct Read6Command read6;
        struct Read10Command read10;
        struct Read12Command read12;
        struct Verify10Command verify10;
        struct Write6Command write6;
        struct Write10Command write10;
        struct Write12Command write12;
      } cb;

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

struct MscQuery
{
  uint64_t position;
  uintptr_t data;
  size_t capacity;
  size_t length;
  size_t offset;
};

struct MscQueryHandler
{
  struct Msc *driver;
  void (*trampoline)(struct Msc *);

  PointerArray usbPool;
  PointerQueue usbQueue;

  size_t currentQueryLength;
  uint64_t currentQueryPosition;
  enum Result currentStatus;

  PointerArray queryPool;
  PointerQueue storageQueries;
  PointerQueue usbQueries;

  /* Preallocated data */
  struct CSW csw;
  struct MscQuery queries[2];
  struct UsbRequest headers[DATA_QUEUE_SIZE];
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_PRIVATE_H_ */
