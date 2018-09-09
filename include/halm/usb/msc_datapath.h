/*
 * halm/usb/msc_datapath.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_USB_MSC_DATAPATH_H_
#define HALM_USB_MSC_DATAPATH_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/usb/msc_defs.h>
#include <halm/usb/usb_request.h>
#include <halm/usb/usb.h>
/*----------------------------------------------------------------------------*/
struct Msc;

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
BEGIN_DECLS

enum Result datapathInit(struct MscQueryHandler *, struct Msc *,
    void (*)(struct Msc *));
void datapathDeinit(struct MscQueryHandler *);
enum Result datapathStatus(const struct MscQueryHandler *);

bool datapathReceiveControl(struct MscQueryHandler *, void *, size_t);
bool datapathSendResponseAndStatus(struct MscQueryHandler *,
    const void *, size_t, uint32_t, uint32_t, uint8_t);
bool datapathSendResponse(struct MscQueryHandler *,
    const void *, size_t);
bool datapathSendStatus(struct MscQueryHandler *,
    uint32_t, uint32_t, uint8_t);
bool datapathReceiveAndWriteData(struct MscQueryHandler *, void *, size_t,
    uint64_t, size_t);
bool datapathReadAndSendData(struct MscQueryHandler *, void *, size_t,
    uint64_t, size_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_DATAPATH_H_ */
