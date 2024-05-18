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

struct Msc;
struct MscQueryHandler;

struct MscConfig
{
  /** Mandatory: USB device. */
  void *device;

  /**
   * Optional: memory region for the temporary buffer. When the pointer is
   * left uninitialized, a memory for the buffer will be allocated on the heap.
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
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result mscAttachUnit(struct Msc *, uint8_t, void *);
void mscDetachUnit(struct Msc *, uint8_t);
bool mscIsUnitFailed(const struct Msc *, uint8_t);
bool mscIsUnitLocked(const struct Msc *, uint8_t);
void mscSetCallback(struct Msc *, void (*)(void *), void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_H_ */
