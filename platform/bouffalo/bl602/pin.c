/*
 * pin.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
#define FUNC_SWGPIO 0xB
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  const unsigned int number = PIN_TO_OFFSET(id);
  const struct Pin pin = {
      .number = (number <= 28) ? number : 0xFF
  };

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  const unsigned int index = pin.number >> 1;
  const unsigned int offset = (pin.number & 1) ? 16 : 0;
  const uint32_t config = GPIO_CFGCTL_IE | GPIO_CFGCTL_SMT
      | GPIO_CFGCTL_FUNC(FUNC_SWGPIO);

  uint32_t cfgctl = BL_GLB->GPIO_CFGCTL[index];

  cfgctl &= ~(GPIO_CFGCTL_MASK << offset);
  cfgctl |= config << offset;

  BL_GLB->GPIO_CFGCTL34 &= ~(1UL << pin.number);
  BL_GLB->GPIO_CFGCTL[index] = cfgctl;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  const unsigned int index = pin.number >> 1;
  const unsigned int offset = (pin.number & 1) ? 16 : 0;
  const uint32_t mask = 1UL << pin.number;
  const uint32_t config = GPIO_CFGCTL_DRV(DRV_LEVEL_3)
      | GPIO_CFGCTL_FUNC(FUNC_SWGPIO);

  uint32_t cfgctl = BL_GLB->GPIO_CFGCTL[index];

  cfgctl &= ~(GPIO_CFGCTL_MASK << offset);
  cfgctl |= config << offset;

  if (value)
    BL_GLB->GPIO_CFGCTL32 |= mask;
  else
    BL_GLB->GPIO_CFGCTL32 &= ~mask;

  BL_GLB->GPIO_CFGCTL[index] = cfgctl;
  BL_GLB->GPIO_CFGCTL34 |= mask;
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  const unsigned int index = pin.number >> 1;
  const unsigned int offset = (pin.number & 1) ? 16 : 0;

  uint32_t cfgctl = BL_GLB->GPIO_CFGCTL[index];
  uint32_t config = (cfgctl >> offset) & GPIO_CFGCTL_MASK;

  config &= ~GPIO_CFGCTL_FUNC_MASK;

  switch (function)
  {
    case PIN_DEFAULT:
      config |= GPIO_CFGCTL_FUNC(FUNC_SWGPIO);
      break;

    default:
      config |= GPIO_CFGCTL_FUNC(function);
      break;
  }

  cfgctl &= ~(GPIO_CFGCTL_MASK << offset);
  cfgctl |= config << offset;

  BL_GLB->GPIO_CFGCTL[index] = cfgctl;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  const unsigned int index = pin.number >> 1;
  const unsigned int offset = (pin.number & 1) ? 16 : 0;

  uint32_t cfgctl = BL_GLB->GPIO_CFGCTL[index];
  uint32_t config = (cfgctl >> offset) & GPIO_CFGCTL_MASK;

  config &= ~(GPIO_CFGCTL_PU | GPIO_CFGCTL_PD);

  switch (pull)
  {
    case PIN_PULLUP:
      config |= GPIO_CFGCTL_PU;
      break;

    case PIN_PULLDOWN:
      config |= GPIO_CFGCTL_PD;
      break;

    default:
      break;
  }

  cfgctl &= ~(GPIO_CFGCTL_MASK << offset);
  cfgctl |= config << offset;

  BL_GLB->GPIO_CFGCTL[index] = cfgctl;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  const unsigned int index = pin.number >> 1;
  const unsigned int offset = (pin.number & 1) ? 16 : 0;

  uint32_t cfgctl = BL_GLB->GPIO_CFGCTL[index];
  uint32_t config = (cfgctl >> offset) & GPIO_CFGCTL_MASK;

  config &= ~GPIO_CFGCTL_DRV_MASK;

  switch (rate)
  {
    case PIN_SLEW_FAST:
      config |= GPIO_CFGCTL_DRV(DRV_LEVEL_3);
      break;

    case PIN_SLEW_NORMAL:
      config |= GPIO_CFGCTL_DRV(DRV_LEVEL_2);
      break;

    default:
      break;
  }

  cfgctl &= ~(GPIO_CFGCTL_MASK << offset);
  cfgctl |= config << offset;

  BL_GLB->GPIO_CFGCTL[index] = cfgctl;
}
