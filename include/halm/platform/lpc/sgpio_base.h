/*
 * halm/platform/lpc/sgpio_base.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SGPIO_BASE_H_
#define HALM_PLATFORM_LPC_SGPIO_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <xcore/entity.h>
/*----------------------------------------------------------------------------*/
enum SgpioPin
{
  SGPIO_0,
  SGPIO_1,
  SGPIO_2,
  SGPIO_3,
  SGPIO_4,
  SGPIO_5,
  SGPIO_6,
  SGPIO_7,
  SGPIO_8,
  SGPIO_9,
  SGPIO_10,
  SGPIO_11,
  SGPIO_12,
  SGPIO_13,
  SGPIO_14,
  SGPIO_15
} __attribute__((packed));

enum SgpioSlice
{
  SGPIO_SLICE_A,
  SGPIO_SLICE_B,
  SGPIO_SLICE_C,
  SGPIO_SLICE_D,
  SGPIO_SLICE_E,
  SGPIO_SLICE_F,
  SGPIO_SLICE_G,
  SGPIO_SLICE_H,
  SGPIO_SLICE_I,
  SGPIO_SLICE_J,
  SGPIO_SLICE_K,
  SGPIO_SLICE_L,
  SGPIO_SLICE_M,
  SGPIO_SLICE_N,
  SGPIO_SLICE_O,
  SGPIO_SLICE_P
} __attribute__((packed));
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SgpioBase;

struct SgpioBase
{
  struct Entity base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

uint8_t sgpioConfigPin(PinNumber, enum PinPull);
uint32_t sgpioGetClock(const struct SgpioBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SGPIO_BASE_H_ */
