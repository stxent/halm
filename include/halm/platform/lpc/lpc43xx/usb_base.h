/*
 * halm/platform/lpc/lpc43xx/usb_base.h
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_LPC43XX_USB_BASE_H_
#define HALM_PLATFORM_LPC_LPC43XX_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/generic/pointer_array.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const UsbBase;

struct QueueHead;
struct TransferDescriptor;

struct UsbBaseConfig
{
  /** Mandatory: USB bidirectional D- line. */
  PinNumber dm;
  /** Mandatory: USB bidirectional D+ line. */
  PinNumber dp;
  /** Mandatory: output pin used for soft connect feature. */
  PinNumber connect;
  /** Mandatory: monitors the presence of USB bus power. */
  PinNumber vbus;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct UsbBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Pointer to an aligned array of Queue Head descriptors */
  struct QueueHead *queueHeads;
  /* Memory allocated for Transfer descriptors */
  struct TransferDescriptor *descriptorMemory;
  /* Pool for transfer descriptors */
  PointerArray descriptorPool;

  /* Unique peripheral identifier */
  uint8_t channel;
  /* Number of logical endpoints */
  uint8_t numberOfEndpoints;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_USB_BASE_H_ */
