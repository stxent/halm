/*
 * gpio.c
 *
 *  Created on: Sep 8, 2012
 *      Author: xen
 */

#include <stdlib.h>
#include "LPC17xx.h"
/* #include "lpc17xx_defines.h" */
#include "gpio.h"
/*------------------Values for function and mode select registers-------------*/
#define PIN_MASK                        0x03
//FIXME
#define PIN_OFFSET(value, offset)       ((value) << (((offset) & 0x0F) << 1))
/*------------------Pin output mode control values----------------------------*/
#define PIN_MODE_PULLUP                 0
#define PIN_MODE_INACTIVE               2
#define PIN_MODE_PULLDOWN               3
/*----------------------------------------------------------------------------*/
static LPC_GPIO_TypeDef *gpioPorts[] = {
    LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4
};
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcPinSelect(union GpioPin);
static inline uint32_t *calcPinMode(union GpioPin);
static inline uint32_t *calcPinModeOD(union GpioPin);
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
void gpioInit()
{
  /* Enable power of the GPIO domain */
  /* LPC_SC->PCONP |= PCONP_PCGPIO; */
}
/*----------------------------------------------------------------------------*/
void gpioDeinit()
{
  /* Disable power of the GPIO domain */
  /* LPC_SC->PCONP &= ~PCONP_PCGPIO; */
}
/*----------------------------------------------------------------------------*/
/* Return 0 when no function associated with id found */
uint8_t gpioFindFunc(const struct GpioPinFunc *pinList, gpioKey key)
{
  while (pinList->key != -1)
  {
    if (pinList->key == key)
      return pinList->func;
    pinList++;
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInitPin(int16_t id, enum gpioDir dir)
{
  uint32_t *pinptr;
  union GpioPin converted;
  struct Gpio p = {
      .control = 0,
      .pin =  {
          .key = -1
      }
  };

  converted.key = id;
  if ((uint8_t)converted.port > 4 || (uint8_t)converted.offset > 31)
    return p;
  p.pin = converted;
  p.control = gpioPorts[p.pin.port];

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

  return p;
}
/*----------------------------------------------------------------------------*/
void gpioReleasePin(struct Gpio *p)
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
}
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *p)
{
  return ((p->control->FIOPIN & (1 << p->pin.offset)) != 0) ? 1 : 0;
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
void gpioSetFunc(struct Gpio *p, uint8_t func)
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
gpioKey gpioGetKey(struct Gpio *p)
{
  return p->pin.key;
}
