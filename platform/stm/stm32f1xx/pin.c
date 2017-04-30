/*
 * pin.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/pin.h>
#include <halm/platform/stm/stm32f1xx/pin_defs.h>
#include <halm/platform/stm/stm32f1xx/system.h>
/*----------------------------------------------------------------------------*/
static inline STM_GPIO_Type *calcPort(struct PinData);
static void commonPinInit(struct PinData);
static inline enum SysBlockReset portToBlockReset(struct PinData);
static inline enum SysClockBranch portToClockBranch(struct PinData);
/*----------------------------------------------------------------------------*/
static inline STM_GPIO_Type *calcPort(struct PinData pin)
{
  return (STM_GPIO_Type *)((uint32_t)STM_GPIOA
      + pin.port * ((uint32_t)STM_GPIOB - (uint32_t)STM_GPIOA));
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct PinData pin)
{
  const enum SysClockBranch branch = portToClockBranch(pin);

  if (!sysClockStatus(branch))
  {
    const enum SysBlockReset reset = portToBlockReset(pin);

    sysClockEnable(branch);
    sysResetEnable(reset);
    sysResetDisable(reset);
  }
}
/*----------------------------------------------------------------------------*/
static inline enum SysBlockReset portToBlockReset(struct PinData pin)
{
  return RST_IOPA + pin.port;
}
/*----------------------------------------------------------------------------*/
static inline enum SysClockBranch portToClockBranch(struct PinData pin)
{
  return CLK_IOPA + pin.port;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  struct Pin pin;

  pin.data.port = PIN_TO_PORT(id);
  pin.data.offset = PIN_TO_OFFSET(id);
  pin.reg = calcPort(pin.data);

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  assert(pin.reg);

  commonPinInit(pin.data);

  const unsigned int index = pin.data.offset >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  /* Configure to floating input */
  configValue &= ~CR_MASK(pin.data.offset);
  configValue |= CR_MODE(pin.data.offset, MODE_INPUT);
  configValue |= CR_CNF(pin.data.offset, CNF_INPUT_FLOATING);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  assert(pin.reg);

  commonPinInit(pin.data);

  const unsigned int index = pin.data.offset >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  /* Configure to push-pull with maximum slew rate */
  configValue &= ~CR_MASK(pin.data.offset);
  configValue |= CR_MODE(pin.data.offset, MODE_OUTPUT_SPEED_50);
  configValue |= CR_CNF(pin.data.offset, CNF_OUTPUT_GP | CNF_OUTPUT_PP);

  pinWrite(pin, value);
  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  const unsigned int index = pin.data.offset >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  uint32_t configuration = CR_CNF_VALUE(pin.data.offset, configValue);
  uint32_t mode = CR_MODE_VALUE(pin.data.offset, configValue);

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

    // TODO Add workaround for remap
  }

  configValue &= ~CR_MASK(pin.data.offset);
  configValue |= CR_MODE(pin.data.offset, mode);
  configValue |= CR_CNF(pin.data.offset, configuration);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  const unsigned int index = pin.data.offset >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  if (CR_MODE_VALUE(pin.data.offset, configValue) != MODE_INPUT)
    return;

  configValue &= ~CR_CNF_VALUE(pin.data.offset, CNF_OUTPUT_MASK);

  if (pull != PIN_NOPULL)
  {
    configValue |= CR_CNF(pin.data.offset, CNF_INPUT_PUPD);
    pinWrite(pin, pull == PIN_PULLUP ? 1 : 0);
  }
  else
    configValue |= CR_CNF(pin.data.offset, CNF_INPUT_FLOATING);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  const unsigned int index = pin.data.offset >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];
  uint32_t rateValue;

  if (CR_MODE_VALUE(pin.data.offset, configValue) == MODE_INPUT)
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

  configValue &= ~CR_MODE_MASK(pin.data.offset);
  configValue |= CR_MODE(pin.data.offset, rateValue);

  reg->CR[index] = configValue;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  const unsigned int index = pin.data.offset >> 3;
  STM_GPIO_Type * const reg = pin.reg;
  uint32_t configValue = reg->CR[index];

  if (CR_MODE_VALUE(pin.data.offset, configValue) == MODE_INPUT)
    return;

  configValue &= ~CR_CNF_VALUE(pin.data.offset, CNF_OUTPUT_MASK);
  configValue |= type == PIN_PUSHPULL ? CNF_OUTPUT_PP : CNF_OUTPUT_OD;

  reg->CR[index] = configValue;
}
