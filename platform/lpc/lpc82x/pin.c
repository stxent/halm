/*
 * pin.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/lpc/lpc82x/pin_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcControlReg(uint8_t);
static void commonPinInit(struct Pin);
static inline bool isI2CPin(struct Pin);
/*----------------------------------------------------------------------------*/
static const int8_t pinFuncMap[29] = {
    -1,  9,  5,  4, 24,  8, 14, 13,
     6,  7, 12, 11, -1, 23, 15, -1,
    -1, 22, 21, 20, 19, 18, 17, 16,
    -1, -1, -1, -1, -1
};

static const uint8_t pinRegMap[29] = {
    0x44, 0x2C, 0x18, 0x14, 0x10, 0x0C, 0x40, 0x3C,
    0x38, 0x34, 0x20, 0x1C, 0x08, 0x04, 0x48, 0x28,
    0x24, 0x00, 0x78, 0x74, 0x70, 0x6C, 0x68, 0x64,
    0x60, 0x5C, 0x58, 0x54, 0x50
};
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcControlReg(uint8_t number)
{
  return (volatile uint32_t *)((uintptr_t)LPC_IOCON + pinRegMap[number]);
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetSlewRate(pin, PIN_SLEW_NORMAL);
  pinSetType(pin, PIN_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
static bool isI2CPin(struct Pin pin)
{
  /* PIO0_10 and PIO0_11 are I2C pins with different function set */
  return pin.number == 10 || pin.number == 11;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin = (struct Pin){NULL, 0, 0};

  if (PIN_TO_PORT(id) == 0)
  {
    pin.number = PIN_TO_OFFSET(id);
    pin.reg = (void *)calcControlReg(pin.number);
  }

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinInit(pin);
  LPC_GPIO->DIR[0] &= ~(1UL << pin.number);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  commonPinInit(pin);
  pinWrite(pin, value);
  LPC_GPIO->DIR[0] |= 1UL << pin.number;
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  if (isI2CPin(pin))
  {
    volatile uint32_t * const reg = pin.reg;
    uint32_t value = *reg & ~IOCON_I2C_MASK;

    value |= function == PIN_DEFAULT ? IOCON_I2C_IO : IOCON_I2C_STANDARD;
    *reg = value;
  }

  uint32_t pinenable = LPC_SWM->PINENABLE[0];
  int8_t cache = -1;
  int8_t offset = pinFuncMap[pin.number];

  /* Disable fixed functions from the first group */
  if (offset >= 0)
  {
    pinenable |= 1UL << offset;

    if (function == PIN_ANALOG)
      cache = offset;
  }

  switch (pin.number)
  {
    case 0:
      offset = 0; /* ACMP_I1 */
      break;
    case 1:
      offset = 1; /* ACMP_I2 */
      break;
    case 6:
      offset = 10; /* VDDCMP */
      break;
    case 14:
      offset = 2; /* ACMP_I3 */
      break;
    case 23:
      offset = 3; /* ACMP_I4 */
      break;
    default:
      offset = -1;
      break;
  }

  /* Disable fixed function from the extended group */
  if (offset >= 0)
  {
    pinenable |= 1UL << offset;

    if (function == PIN_ANALOG_ACMP)
      cache = offset;
  }

  switch (function)
  {
    case PIN_DEFAULT:
      break;

    case PIN_ANALOG:
    case PIN_ANALOG_ACMP:
      pinenable &= ~(1UL << cache);
      break;

    default:
      pinenable &= ~(1UL << function);
      break;
  }

  LPC_SWM->PINENABLE[0] = pinenable;
}
/*----------------------------------------------------------------------------*/
void pinSetMux(struct Pin pin, enum PinMuxIndex mux)
{
  const unsigned int index = mux >> 2;
  const unsigned int offset = mux & 0x3;
  uint32_t value = LPC_SWM->PINASSIGN[index];

  value &= ~PINASSIGN_MUX_MASK(offset);
  value |= PINASSIGN_MUX(offset, pin.number);

  LPC_SWM->PINASSIGN[index] = value;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  if (isI2CPin(pin))
    return;

  volatile uint32_t * const reg = pin.reg;
  uint32_t value = *reg & ~IOCON_MODE_MASK;

  switch (pull)
  {
    case PIN_NOPULL:
      value |= IOCON_MODE_INACTIVE;
      break;

    case PIN_PULLUP:
      value |= IOCON_MODE_PULLUP;
      break;

    case PIN_PULLDOWN:
      value |= IOCON_MODE_PULLDOWN;
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  /* Slew rate control is available only for I2C pins */
  if (!isI2CPin(pin))
    return;

  volatile uint32_t * const reg = pin.reg;
  uint32_t value = *reg & ~IOCON_I2C_MASK;

  value |= (rate == PIN_SLEW_FAST) ? IOCON_I2C_PLUS : IOCON_I2C_STANDARD;
  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  if (isI2CPin(pin))
    return;

  volatile uint32_t * const reg = pin.reg;

  switch (type)
  {
    case PIN_PUSHPULL:
      *reg &= ~IOCON_OD;
      break;

    case PIN_OPENDRAIN:
      *reg |= IOCON_OD;
      break;
  }
}
