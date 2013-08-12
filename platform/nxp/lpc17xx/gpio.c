/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "macro.h"
#include "platform/gpio.h"
#include "platform/nxp/device_defs.h"
//#include "platform/nxp/lpc13xx/power.h"
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
  return (LPC_GPIO_TypeDef *)((void *)LPC_GPIO0
      + ((void *)LPC_GPIO1 - (void *)LPC_GPIO0) * p.port);
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
/* Returns 0 when no descriptor associated with pin found */
const struct GpioDescriptor *gpioFind(const struct GpioDescriptor *list,
    gpioKey key, uint8_t channel)
{
  while (list->key && (list->key != key || list->channel != channel))
    ++list;

  return list->key ? list : 0;
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpioKey id, enum gpioDir dir)
{
  struct Gpio p = {
      .reg = 0,
      .pin = {
          .key = ~0
      }
  };
  union GpioPin converted = {
      .key = ~id /* Invert unique pin id */
  };

  /* TODO Add more precise pin checking */
  assert(id && (uint8_t)converted.port <= 4 && (uint8_t)converted.offset <= 31);

  p.pin = converted;
  p.reg = calcPort(p.pin);

  /* Set function 0: GPIO mode for all LPC17xx parts */
  gpioSetFunction(&p, 0);
  /* Set mode 2: neither pull-up nor pull-down */
  gpioSetPull(&p, GPIO_NOPULL);
  /* Set mode 0: normal mode (not open drain) */
  gpioSetType(&p, GPIO_PUSHPULL);

  if (dir == GPIO_OUTPUT)
    ((LPC_GPIO_TypeDef *)&p.reg)->FIODIR |= 1 << p.pin.offset;
  else
    ((LPC_GPIO_TypeDef *)&p.reg)->FIODIR &= ~(1 << p.pin.offset);

  /* TODO Add default output value */

  /* There is no need to enable GPIO power because it is enabled on reset */
  return p;
}
/*----------------------------------------------------------------------------*/
void gpioDeinit(struct Gpio *p)
{
  ((LPC_GPIO_TypeDef *)p->reg)->FIODIR &= ~(1 << p->pin.offset);
  /* Reset values to default (0) */
  gpioSetType(p, 0);
  gpioSetPull(p, 0);
  gpioSetFunction(p, 0);

  /* TODO Check possibility of disabling power when no pins are used */
}
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *p)
{
  return (((LPC_GPIO_TypeDef *)p->reg)->FIOPIN & (1 << p->pin.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
void gpioWrite(struct Gpio *p, uint8_t value)
{
  if (value)
    ((LPC_GPIO_TypeDef *)p->reg)->FIOSET = 1 << p->pin.offset;
  else
    ((LPC_GPIO_TypeDef *)p->reg)->FIOCLR = 1 << p->pin.offset;
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio *p, uint8_t function)
{
  uint32_t *pinptr = calcPinSelect(p->pin);

  *pinptr &= ~PIN_OFFSET(PIN_MASK, p->pin.offset);
  *pinptr |= PIN_OFFSET(function, p->pin.offset);
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
