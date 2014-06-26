/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdbool.h>
#include <entity.h>
#include <gpio.h>
#include <platform/nxp/lpc13xx/gpio_defs.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
struct GpioHandler
{
  struct Entity parent;

  /* Initialized pins count */
  uint8_t instances;
};
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union GpioPin);
static inline volatile uint32_t *calcControlReg(union GpioPin);
static void commonGpioSetup(struct Gpio);
/*----------------------------------------------------------------------------*/
static inline void gpioHandlerAttach();
static inline void gpioHandlerDetach();
static enum result gpioHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct GpioHandler),
    .init = gpioHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
static const uint8_t gpioRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const GpioHandler = &handlerTable;
static struct GpioHandler *gpioHandler = 0;
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union GpioPin pin)
{
  return (void *)(((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * pin.port
      + (uint32_t)LPC_GPIO0);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcControlReg(union GpioPin pin)
{
  return (volatile uint32_t *)((uint32_t)LPC_IOCON
      + gpioRegMap[pin.port][pin.offset]);
}
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio gpio)
{
  /* Register new pin in the handler */
  gpioHandlerAttach();

  gpioSetFunction(gpio, GPIO_DEFAULT);
  gpioSetPull(gpio, GPIO_NOPULL);
  gpioSetType(gpio, GPIO_PUSHPULL);
}
/*----------------------------------------------------------------------------*/
static inline void gpioHandlerAttach()
{
  /* Create handler object on first function call */
  if (!gpioHandler)
    gpioHandler = init(GpioHandler, 0);

  if (!gpioHandler->instances++)
  {
    sysClockEnable(CLK_IOCON);
    sysClockEnable(CLK_GPIO);
  }
}
/*----------------------------------------------------------------------------*/
static inline void gpioHandlerDetach()
{
  /* Disable clocks when no active pins exist */
  if (!--gpioHandler->instances)
  {
    sysClockDisable(CLK_GPIO);
    sysClockDisable(CLK_IOCON);
  }
}
/*----------------------------------------------------------------------------*/
static enum result gpioHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct GpioHandler * const handler = object;

  handler->instances = 0;
  return E_OK;
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
  volatile uint32_t * const iocon = calcControlReg(gpio.pin);
  const uint32_t value = *iocon;

  switch (function)
  {
    case GPIO_DEFAULT:
      /* Some pins have default function value other than zero */
      function = (gpio.pin.port == 1 && gpio.pin.offset <= 2)
          || (gpio.pin.port == 0 && gpio.pin.offset == 11) ? 1 : 0;
      break;

    case GPIO_ANALOG:
      *iocon = value & ~IOCON_MODE_DIGITAL;
      return;
  }

  *iocon = (value & ~IOCON_FUNC_MASK) | IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio gpio, enum gpioPull pull)
{
  volatile uint32_t * const iocon = calcControlReg(gpio.pin);
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
  volatile uint32_t * const iocon = calcControlReg(gpio.pin);

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
