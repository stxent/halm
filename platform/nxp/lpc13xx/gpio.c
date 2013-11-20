/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <gpio.h>
#include <platform/nxp/lpc13xx/gpio_defs.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(union GpioPin);
static inline void *calcReg(union GpioPin p);
/*----------------------------------------------------------------------------*/
static void blockSetEnabled(bool);
static void commonGpioSetup(struct Gpio);
/*----------------------------------------------------------------------------*/
static const uint8_t gpioRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
/* Initialized pins count */
static uint8_t instances = 0;
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(union GpioPin p)
{
  return (LPC_GPIO_Type *)((uint32_t)LPC_GPIO0 +
      ((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * p.port);
}
/*----------------------------------------------------------------------------*/
static inline void *calcReg(union GpioPin p)
{
  return (void *)((uint32_t)LPC_IOCON + (uint32_t)gpioRegMap[p.port][p.offset]);
}
/*----------------------------------------------------------------------------*/
static void blockSetEnabled(bool state)
{
  if (state)
  {
    /* Enable clock to IO configuration block */
    sysClockEnable(CLK_IOCON);
    /* Enable AHB clock to the GPIO domain */
    sysClockEnable(CLK_GPIO);
  }
  else
  {
    /* Disable all IO clocks */
    sysClockDisable(CLK_GPIO);
    sysClockDisable(CLK_IOCON);
  }
}
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio gpio)
{
  if (!instances++)
    blockSetEnabled(true);

  /* Set GPIO mode */
  gpioSetFunction(gpio, GPIO_DEFAULT);
  /* Neither pull-up nor pull-down */
  gpioSetPull(gpio, GPIO_NOPULL);
  /* Push-pull output type */
  gpioSetType(gpio, GPIO_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpio_t id)
{
  struct Gpio gpio;

  gpio.pin.key = ~id;
  gpio.reg = calcPort(gpio.pin);

  return gpio;
}
/*----------------------------------------------------------------------------*/
void gpioInput(struct Gpio gpio)
{
  commonGpioSetup(gpio);
  ((LPC_GPIO_Type *)gpio.reg)->DIR &= ~(1 << gpio.pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioOutput(struct Gpio gpio, uint8_t value)
{
  commonGpioSetup(gpio);
  ((LPC_GPIO_Type *)gpio.reg)->DIR |= 1 << gpio.pin.offset;
  gpioWrite(gpio, value);
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio gpio, uint8_t function)
{
  uint32_t *iocon = calcReg(gpio.pin);
  uint32_t value = *iocon;

  switch (function)
  {
    case GPIO_DEFAULT:
      function = (gpio.pin.port == 1 && gpio.pin.offset <= 2)
          || (gpio.pin.port == 0 && gpio.pin.offset == 11) ? 1 : 0;
      break;
    case GPIO_ANALOG:
      *iocon = value & ~IOCON_MODE_DIGITAL;
      return;
  }

  *iocon = (value & ~IOCON_FUNC_MASK) | IOCON_MODE_DIGITAL
      | IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio gpio, enum gpioPull pull)
{
  uint32_t *iocon = calcReg(gpio.pin);
  uint32_t value = *iocon & ~IOCON_MODE_MASK;

  switch (pull)
  {
    case GPIO_NOPULL:
      value |= IOCON_MODE_INACTIVE;
      break;
    case GPIO_PULLUP:
      value |= IOCON_MODE_PULLUP;
      break;
    case GPIO_PULLDOWN:
      value |= IOCON_MODE_PULLDOWN;
      break;
  }
  *iocon = value;
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio gpio, enum gpioType type)
{
  uint32_t *iocon = calcReg(gpio.pin);

  switch (type)
  {
    case GPIO_PUSHPULL:
      *iocon &= ~IOCON_OD;
      break;
    case GPIO_OPENDRAIN:
      *iocon |= IOCON_OD;
      break;
  }
}
