/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "gpio.h"
/*------------------Values for function and mode select registers-------------*/
#define PIN_MASK                        0x03
#define PIN_OFFSET(value, offset) \
    ((uint32_t)((value) << (((offset) & 0x0F) << 1)))
/*------------------Pin output mode control values----------------------------*/
#define PIN_MODE_PULLUP                 0
#define PIN_MODE_INACTIVE               2
#define PIN_MODE_PULLDOWN               3
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_TypeDef *calcPort(union GpioPin);
static inline uint32_t *calcPinSelect(union GpioPin);
static inline uint32_t *calcPinMode(union GpioPin);
static inline uint32_t *calcPinModeOD(union GpioPin);
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_TypeDef *calcPort(union GpioPin p)
{
  return (LPC_GPIO_TypeDef *)((void *)LPC_GPIO0 +
      ((void *)LPC_GPIO1 - (void *)LPC_GPIO0) * p.port);
}
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinSelect(union GpioPin p)
{
  return (uint32_t *)(&LPC_PINCON->PINSEL0 + (p.offset >> 4) + (p.port << 1));
}
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinMode(union GpioPin p)
{
  return (uint32_t *)(&LPC_PINCON->PINMODE0 + (p.offset >> 4) + (p.port << 1));
}
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinModeOD(union GpioPin p)
{
  return (uint32_t *)(&LPC_PINCON->PINMODE_OD0 + p.port);
}
/*----------------------------------------------------------------------------*/
/* Returns -1 when no function associated with pin found */
gpioFunc gpioFindFunc(const struct GpioPinFunc *pinList, gpioKey key)
{
  while (pinList->key)
  {
    if (pinList->key == key)
      return pinList->func;
    pinList++;
  }
  return -1;
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpioKey id, enum gpioDir dir)
{
  uint32_t *pinptr;
  union GpioPin converted = {
      .key = ~id /* Invert unique pin id */
  };
  struct Gpio p = {
      .control = 0,
      .pin = {
          .key = ~0
      }
  };

  /* TODO Add more precise pin checking */
  if (!id || (uint8_t)converted.port > 4 || (uint8_t)converted.offset > 31)
    return p;
  p.pin = converted;
  p.control = calcPort(p.pin);

  /* Calculate PINSEL register */
  pinptr = calcPinSelect(p.pin);
  /* Set function 0: GPIO mode */
  *pinptr &= ~PIN_OFFSET(PIN_MASK, p.pin.offset);

  /* Calculate PINMODE register */
  pinptr = calcPinMode(p.pin);
  /* Set mode 2: neither pull-up nor pull-down */
  *pinptr &= ~PIN_OFFSET(PIN_MASK, p.pin.offset);
  *pinptr |= PIN_OFFSET(PIN_MODE_INACTIVE, p.pin.offset);

  /* Calculate PINMODE_OD register */
  pinptr = calcPinModeOD(p.pin);
  /* Set mode 0: normal mode (not open drain) */
  *pinptr &= ~(1 << p.pin.offset);

  if (dir == GPIO_OUTPUT)
    p.control->FIODIR |= 1 << p.pin.offset;
  else
    p.control->FIODIR &= ~(1 << p.pin.offset);

  /* There is no need to enable GPIO power because it is enabled on reset */
  return p;
}
/*----------------------------------------------------------------------------*/
void gpioDeinit(struct Gpio *p)
{
  uint32_t *pinptr;

  p->control->FIODIR &= ~(1 << p->pin.offset);
  /* Reset values to default (0) */
  /* Calculate PINSEL register */
  pinptr = calcPinSelect(p->pin);
  *pinptr &= ~PIN_OFFSET(PIN_MASK, p->pin.offset);

  /* Calculate PINMODE register */
  pinptr = calcPinMode(p->pin);
  *pinptr &= ~PIN_OFFSET(PIN_MASK, p->pin.offset);

  /* Calculate PINMODE_OD register */
  pinptr = calcPinModeOD(p->pin);
  *pinptr &= ~(1 << p->pin.offset);

  /* TODO Check possibility of disabling power when no pins are used */
  /* LPC_SC->PCONP &= ~PCONP_PCGPIO; */
}
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *p)
{
  return (p->control->FIOPIN & (1 << p->pin.offset)) != 0 ? 1 : 0;
}
/*----------------------------------------------------------------------------*/
void gpioWrite(struct Gpio *p, uint8_t value)
{
  if (value)
    p->control->FIOSET |= 1 << p->pin.offset;
  else
    p->control->FIOCLR |= 1 << p->pin.offset;
}
/*----------------------------------------------------------------------------*/
void gpioSetFunc(struct Gpio *p, gpioFunc func)
{
  uint32_t *pinptr = calcPinSelect(p->pin);
  *pinptr &= ~PIN_OFFSET(PIN_MASK, p->pin.offset);
  *pinptr |= PIN_OFFSET(func & 0x03, p->pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio *p, enum gpioPull pull)
{
  uint32_t *pinptr = calcPinMode(p->pin);
  *pinptr &= ~PIN_OFFSET(PIN_MASK, p->pin.offset);
  switch (pull)
  {
    case GPIO_NOPULL:
      *pinptr |= PIN_OFFSET(PIN_MODE_INACTIVE, p->pin.offset);
      break;
    case GPIO_PULLUP:
      *pinptr |= PIN_OFFSET(PIN_MODE_PULLUP, p->pin.offset);
      break;
    case GPIO_PULLDOWN:
      *pinptr |= PIN_OFFSET(PIN_MODE_PULLDOWN, p->pin.offset);
      break;
  }
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio *p, enum gpioType type)
{
  uint32_t *pinptr = calcPinModeOD(p->pin);
  switch (type)
  {
    case GPIO_PUSHPULL:
      *pinptr &= ~(1 << p->pin.offset);
      break;
    case GPIO_OPENDRAIN:
      *pinptr |= 1 << p->pin.offset;
      break;
  }
}
/*----------------------------------------------------------------------------*/
/* Returns zero when pin not initialized */
gpioKey gpioGetKey(struct Gpio *p)
{
  return ~p->pin.key; /* External pin identifiers are in 1's complement form */
}
