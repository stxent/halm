/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <gpio.h>
#include <platform/nxp/lpc17xx/gpio_defs.h>
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union GpioPin);
static inline volatile uint32_t *calcPinSelect(union GpioPin);
static inline volatile uint32_t *calcPinMode(union GpioPin);
static inline volatile uint32_t *calcPinType(union GpioPin);
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio);
/*----------------------------------------------------------------------------*/
static inline void *calcPort(union GpioPin pin)
{
  return (void *)(((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * pin.port
      + (uint32_t)LPC_GPIO0);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinSelect(union GpioPin pin)
{
  return &LPC_PINCON->PINSEL0 + (pin.offset >> 4) + (pin.port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinMode(union GpioPin pin)
{
  return &LPC_PINCON->PINMODE0 + (pin.offset >> 4) + (pin.port << 1);
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcPinType(union GpioPin pin)
{
  return &LPC_PINCON->PINMODE_OD0 + pin.port;
}
/*----------------------------------------------------------------------------*/
static void commonGpioSetup(struct Gpio gpio)
{
  gpioSetFunction(gpio, GPIO_DEFAULT);
  gpioSetPull(gpio, GPIO_NOPULL);
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
  /* Function should not be used outside platform drivers */
  switch (function)
  {
    case GPIO_DEFAULT:
      function = 0; /* Zero function is the default for all pins */
      break;

    case GPIO_ANALOG:
      return;
  }

  volatile uint32_t * const reg = calcPinSelect(gpio.pin);

  *reg = (*reg & ~PIN_OFFSET(PIN_MASK, gpio.pin.offset))
      | PIN_OFFSET(function, gpio.pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio gpio, enum gpioPull pull)
{
  volatile uint32_t * const reg = calcPinMode(gpio.pin);
  uint32_t value = *reg & ~PIN_OFFSET(PIN_MASK, gpio.pin.offset);

  switch (pull)
  {
    case GPIO_NOPULL:
      value |= PIN_OFFSET(PIN_MODE_INACTIVE, gpio.pin.offset);
      break;

    case GPIO_PULLUP:
      value |= PIN_OFFSET(PIN_MODE_PULLUP, gpio.pin.offset);
      break;

    case GPIO_PULLDOWN:
      value |= PIN_OFFSET(PIN_MODE_PULLDOWN, gpio.pin.offset);
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio gpio, enum gpioType type)
{
  volatile uint32_t * const reg = calcPinType(gpio.pin);
  uint32_t value = *reg;

  switch (type)
  {
    case GPIO_PUSHPULL:
      value &= ~(1 << gpio.pin.offset);
      break;

    case GPIO_OPENDRAIN:
      value |= 1 << gpio.pin.offset;
      break;
  }

  *reg = value;
}
