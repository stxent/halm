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
enum [[gnu::packed]] SgpioPin
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
};

enum [[gnu::packed]] SgpioSlice
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
};
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

static inline int8_t sgpioSliceToClockSource(enum SgpioSlice slice)
{
  if (slice == SGPIO_SLICE_D)
    return 0;

  if (slice == SGPIO_SLICE_H)
    return 1;

  if (slice == SGPIO_SLICE_O || slice == SGPIO_SLICE_P)
    return slice - 12;

  return -1;
}

static inline int8_t sgpioSliceToQualifierSource(enum SgpioSlice source,
    enum SgpioSlice destination)
{
  if (destination == SGPIO_SLICE_A && source == SGPIO_SLICE_D)
    return 0;
  if (destination == SGPIO_SLICE_H && source == SGPIO_SLICE_O)
    return 1;
  if (destination == SGPIO_SLICE_I && source == SGPIO_SLICE_D)
    return 2;
  if (destination == SGPIO_SLICE_P && source == SGPIO_SLICE_O)
    return 3;

  switch (source)
  {
    case SGPIO_SLICE_A:
      return 0;

    case SGPIO_SLICE_H:
      return 1;

    case SGPIO_SLICE_I:
      return 2;

    case SGPIO_SLICE_P:
      return 3;

    default:
      return -1;
  }
}

END_DECLS
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum SgpioPin sgpioConfigPin(PinNumber, enum PinPull);
uint32_t sgpioGetClock(const struct SgpioBase *);
enum SgpioSlice sgpioPinToSlice(enum SgpioPin, uint8_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SGPIO_BASE_H_ */
