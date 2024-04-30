/*
 * halm/platform/imxrt/usb_base.h
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_IMXRT_USB_BASE_H_
#define HALM_PLATFORM_IMXRT_USB_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/core/cortex/mpu.h>
#include <halm/generic/pointer_array.h>
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/usb_base.h>
#include HEADER_PATH
#undef HEADER_PATH
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
  /** Mandatory: monitors the presence of USB bus power. */
  PinNumber vbus;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
};

struct UsbBase
{
  struct Entity base;

  void *phy;
  void *reg;
  void (*handler)(void *);

  struct
  {
    IrqNumber phy;
    IrqNumber usb;
  } irq;

  /* MPU region number */
  MpuRegion region;
  /* Unique peripheral identifier */
  uint8_t channel;

  struct
  {
    /* Pointer to an aligned array of Queue Head descriptors */
    struct QueueHead *heads;
    /* Pool for transfer descriptors */
    PointerArray descriptors;
    /* Physical endpoint count */
    uint8_t numberOfEndpoints;
  } td;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_IMXRT_USB_BASE_H_ */
