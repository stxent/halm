/*
 * pin.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/stm32/stm32f1xx/pin_defs.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static inline STM_GPIO_Type *calcPort(uint8_t);
static void commonPinInit(struct Pin pin);
static inline enum SysBlockReset portToBlockReset(uint8_t);
static inline enum SysClockBranch portToClockBranch(uint8_t);
/*----------------------------------------------------------------------------*/
static const struct PinEntry swjPins[] = {
    {
        .key = PIN(PORT_A, 13), /* JTMS/SWDIO */
        .channel = 0,
        .value = PACK_REMAP(REMAP_SWJ, 4)
    }, {
        .key = PIN(PORT_A, 14), /* JTCK/SWCLK */
        .channel = 0,
        .value = PACK_REMAP(REMAP_SWJ, 4)
    }, {
        .key = PIN(PORT_A, 15), /* JTDI */
        .channel = 0,
        .value = PACK_REMAP(REMAP_SWJ, 2)
    }, {
        .key = PIN(PORT_B, 3), /* JTDO/SWO */
        .channel = 0,
        .value = PACK_REMAP(REMAP_SWJ, 2)
    }, {
        .key = PIN(PORT_B, 4), /* NJTRST */
        .channel = 0,
        .value = PACK_REMAP(REMAP_SWJ, 1)
    }, {
        .key = 0
    }
};
/*----------------------------------------------------------------------------*/
static inline STM_GPIO_Type *calcPort(uint8_t port)
{
  return (STM_GPIO_Type *)((uint32_t)STM_GPIOA
      + port * ((uint32_t)STM_GPIOB - (uint32_t)STM_GPIOA));
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  const enum SysClockBranch branch = portToClockBranch(pin.port);

  if (!sysClockStatus(branch))
  {
    const enum SysBlockReset reset = portToBlockReset(pin.port);

    sysClockEnable(branch);
    sysResetEnable(reset);
    sysResetDisable(reset);
  }

  pinSetFunction(pin, PIN_DEFAULT);
}
/*----------------------------------------------------------------------------*/
static inline enum SysBlockReset portToBlockReset(uint8_t port)
{
  return RST_GPIOA + port;
}
/*----------------------------------------------------------------------------*/
static inline enum SysClockBranch portToClockBranch(uint8_t port)
{
  return CLK_GPIOA + port;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.port = PIN_TO_PORT(id);
  pin.number = PIN_TO_OFFSET(id);
  pin.mask = id ? (1 << pin.number) : 0;
  pin.reg = id ? calcPort(pin.port) : 0;

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  assert(pinValid(pin));
  commonPinInit(pin);

  const unsigned int index = pin.number >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  /* Configure to floating input */
  configValue &= ~CR_MASK(pin.number);
  configValue |= CR_MODE(pin.number, MODE_INPUT);
  configValue |= CR_CNF(pin.number, CNF_INPUT_FLOATING);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  assert(pinValid(pin));
  commonPinInit(pin);

  const unsigned int index = pin.number >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  /* Enable push-pull output with maximum slew rate */
  configValue &= ~CR_MASK(pin.number);
  configValue |= CR_MODE(pin.number, MODE_OUTPUT_SPEED_50);
  configValue |= CR_CNF(pin.number, CNF_OUTPUT_GP | CNF_OUTPUT_PP);

  pinWrite(pin, value);
  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  const unsigned int index = pin.number >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  uint32_t configuration = CR_CNF_VALUE(pin.number, configValue);
  uint32_t mode = CR_MODE_VALUE(pin.number, configValue);

  if (function == PIN_DEFAULT)
  {
    if (mode == MODE_INPUT)
    {
      /* Reconfigure to floating input */
      if (configuration == CNF_INPUT_ANALOG)
        configuration = CNF_INPUT_FLOATING;
    }
    else
    {
      /* Reconfigure to general-purpose output preserving PP/OD settings */
      if ((configuration & CNF_FUNCTION_MASK) == CNF_OUTPUT_AF)
        configuration = CNF_OUTPUT_GP;
    }

    const struct PinEntry * const swjPinEntry =
        pinFind(swjPins, PIN(pin.port, pin.number), 0);

    if (swjPinEntry != NULL)
      pinSetFunction(pin, swjPinEntry->value);
  }
  else if (function == PIN_ANALOG)
  {
    /* Reconfigure to analog input */
    mode = MODE_INPUT;
    configuration = CNF_INPUT_ANALOG;
  }
  else
  {
    if (mode != MODE_INPUT)
      configuration |= CNF_FUNCTION_MASK;

    if (function)
      pinRemapApply(function);
  }

  configValue &= ~CR_MASK(pin.number);
  configValue |= CR_MODE(pin.number, mode);
  configValue |= CR_CNF(pin.number, configuration);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  const unsigned int index = pin.number >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  if (CR_MODE_VALUE(pin.number, configValue) != MODE_INPUT)
    return;

  configValue &= ~CR_CNF_MASK(pin.number);

  if (pull != PIN_NOPULL)
  {
    configValue |= CR_CNF(pin.number, CNF_INPUT_PUPD);
    pinWrite(pin, pull == PIN_PULLUP ? 1 : 0);
  }
  else
    configValue |= CR_CNF(pin.number, CNF_INPUT_FLOATING);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  const unsigned int index = pin.number >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];
  uint32_t rateValue;

  if (CR_MODE_VALUE(pin.number, configValue) == MODE_INPUT)
    return;

  switch (rate)
  {
    case PIN_SLEW_FAST:
      rateValue = MODE_OUTPUT_SPEED_50;
      break;

    case PIN_SLEW_NORMAL:
      rateValue = MODE_OUTPUT_SPEED_10;
      break;

    default:
      rateValue = MODE_OUTPUT_SPEED_2;
      break;
  }

  configValue &= ~CR_MODE_MASK(pin.number);
  configValue |= CR_MODE(pin.number, rateValue);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  const unsigned int index = pin.number >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  if (CR_MODE_VALUE(pin.number, configValue) == MODE_INPUT)
    return;

  configValue &= ~CR_CNF_VALUE(pin.number, CNF_OUTPUT_MASK);
  configValue |= type == PIN_PUSHPULL ? CNF_OUTPUT_PP : CNF_OUTPUT_OD;

  reg->CR[index] = configValue;
}
