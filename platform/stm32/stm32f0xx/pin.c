/*
 * pin.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/stm32/stm32f0xx/pin_defs.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline STM_GPIO_Type *calcPort(uint8_t);
static void commonPinInit(uint8_t);
static inline enum SysBlockReset portToBlockReset(uint8_t);
static inline enum SysClockBranch portToClockBranch(uint8_t);
/*----------------------------------------------------------------------------*/
static inline STM_GPIO_Type *calcPort(uint8_t port)
{
  return (STM_GPIO_Type *)((uint32_t)STM_GPIOA
      + port * ((uint32_t)STM_GPIOB - (uint32_t)STM_GPIOA));
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(uint8_t port)
{
  const enum SysClockBranch branch = portToClockBranch(port);

  if (!sysClockStatus(branch))
  {
    const enum SysBlockReset reset = portToBlockReset(port);

    sysClockEnable(branch);
    sysResetEnable(reset);
    sysResetDisable(reset);
  }
}
/*----------------------------------------------------------------------------*/
static inline enum SysBlockReset portToBlockReset(uint8_t port)
{
  return RST_IOPA + port;
}
/*----------------------------------------------------------------------------*/
static inline enum SysClockBranch portToClockBranch(uint8_t port)
{
  return CLK_IOPA + port;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.port = PIN_TO_PORT(id);
  pin.number = PIN_TO_OFFSET(id);
  pin.mask = 1UL << pin.number;
  pin.reg = id ? calcPort(pin.port) : 0;

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  assert(pin.reg);
  commonPinInit(pin.port);

  STM_GPIO_Type * const reg = pin.reg;
  uint32_t moder = reg->MODER;

  /* Configure to floating input */
  moder &= ~MODER_MODE_MASK(pin.number);
  moder |= MODER_MODE(pin.number, MODER_INPUT);
  reg->MODER = moder;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  assert(pin.reg);
  commonPinInit(pin.port);

  STM_GPIO_Type * const reg = pin.reg;
  uint32_t moder = reg->MODER;

  /* Configure to push-pull output */
  moder &= ~MODER_MODE_MASK(pin.number);
  moder |= MODER_MODE(pin.number, MODER_OUTPUT);
  reg->MODER = moder;

  pinSetType(pin, PIN_PUSHPULL);
  pinSetPull(pin, PIN_NOPULL);
  pinSetSlewRate(pin, PIN_SLEW_FAST);

  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t value;

  if (function == PIN_DEFAULT)
  {
    /* Initial state is unknown, reconfigure to floating input */
    value = MODER_INPUT;
  }
  else if (function == PIN_ANALOG)
  {
    value = MODER_ANALOG;
  }
  else
  {
    const unsigned int index = pin.number >> 3;
    uint32_t afr = reg->AFR[index];

    afr &= ~AFR_AFSEL_MASK(pin.number);
    afr |= AFR_AFSEL(pin.number, function);
    reg->AFR[index] = afr;

    value = MODER_AF;
  }

  const uint32_t moder = reg->MODER & ~MODER_MODE_MASK(pin.number);
  reg->MODER = moder | MODER_MODE(pin.number, value);
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t pupdr = reg->PUPDR;
  uint32_t value;

  switch (pull)
  {
    case PIN_PULLUP:
      value = PUPDR_PULLUP;
      break;

    case PIN_PULLDOWN:
      value = PUPDR_PULLDOWN;
      break;

    default:
      value = PUPDR_NOPULL;
      break;
  }

  pupdr &= ~PUPDR_PULL_MASK(pin.number);
  pupdr |= PUPDR_PULL(pin.number, value);

  reg->PUPDR = pupdr;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t ospeedr = reg->OSPEEDR;
  uint32_t value;

  switch (rate)
  {
    case PIN_SLEW_SLOW:
      value = OSPEEDR_LOW;
      break;

    case PIN_SLEW_NORMAL:
      value = OSPEEDR_MEDIUM;
      break;

    default:
      value = OSPEEDR_HIGH;
      break;
  }

  ospeedr &= ~OSPEEDR_SPEED_MASK(pin.number);
  ospeedr |= OSPEEDR_SPEED(pin.number, value);

  reg->OSPEEDR = ospeedr;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  STM_GPIO_Type * const reg = pin.reg;
  const uint32_t value = type == PIN_OPENDRAIN ? OTYPER_OD : OTYPER_PP;
  uint32_t otyper = reg->OTYPER;

  otyper &= ~BIT(pin.number);
  otyper |= value << pin.number;

  reg->OTYPER = otyper;
}
