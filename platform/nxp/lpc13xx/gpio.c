/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <gpio.h>
#include <macro.h>
#include <platform/nxp/lpc13xx/gpio_defs.h>
#include <platform/nxp/lpc13xx/power.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(union GpioPin);
static inline void *calcReg(union GpioPin p);
/*----------------------------------------------------------------------------*/
/* IOCON register map differs for LPC1315/16/17/45/46/47 */
static const uint8_t gpioRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
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
/* Returns 0 when no descriptor associated with pin found */
const struct GpioDescriptor *gpioFind(const struct GpioDescriptor *list,
    gpio_t key, uint8_t channel)
{
  while (list->key && (list->key != key || list->channel != channel))
    ++list;

  return list->key ? list : 0;
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInit(gpio_t id, enum gpioDir dir)
{
  uint32_t *iocon;
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
  assert(id && (uint8_t)converted.port <= 3 && (uint8_t)converted.offset <= 11);

  p.pin = converted;
  p.reg = calcPort(p.pin);

  iocon = calcReg(p.pin);
  /* PIO function, no pull, no hysteresis, standard output */
  *iocon = IOCON_DEFAULT & ~IOCON_MODE_MASK;

  /* Exceptions */
  if ((p.pin.port == 1 && p.pin.offset <= 2)
      || (p.pin.port == 0 && p.pin.offset == 11))
  {
    *iocon |= 0x01; /* Select GPIO function */
  }
  /* TODO Add analog functions */

  if (dir == GPIO_OUTPUT)
    ((LPC_GPIO_Type *)p.reg)->DIR |= 1 << p.pin.offset;
  else
    ((LPC_GPIO_Type *)p.reg)->DIR &= ~(1 << p.pin.offset);

  if (!instances++)
  {
    /* Enable clock to IO configuration block */
    sysClockEnable(CLK_IOCON);
    /* Enable AHB clock to the GPIO domain */
    sysClockEnable(CLK_GPIO);
  }

  return p;
}
/*----------------------------------------------------------------------------*/
void gpioDeinit(struct Gpio *p)
{
  uint32_t *iocon = calcReg(p->pin);

  ((LPC_GPIO_Type *)p->reg)->DIR &= ~(1 << p->pin.offset);
  *iocon = IOCON_DEFAULT;

  if (!--instances)
  {
    /* Disable AHB clock to the GPIO domain */
    sysClockDisable(CLK_GPIO);
    /* Disable clock to IO configuration block */
    sysClockDisable(CLK_IOCON);
  }
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio *p, uint8_t function)
{
  uint32_t *iocon = calcReg(p->pin);

  *iocon &= ~IOCON_FUNC_MASK;
  *iocon |= IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio *p, enum gpioPull pull)
{
  uint32_t *iocon = calcReg(p->pin);

  switch (pull)
  {
    case GPIO_NOPULL:
      *iocon = (*iocon & ~IOCON_MODE_MASK) | IOCON_MODE_INACTIVE;
      break;
    case GPIO_PULLUP:
      *iocon = (*iocon & ~IOCON_MODE_MASK) | IOCON_MODE_PULLUP;
      break;
    case GPIO_PULLDOWN:
      *iocon = (*iocon & ~IOCON_MODE_MASK) | IOCON_MODE_PULLDOWN;
      break;
  }
}
/*----------------------------------------------------------------------------*/
void gpioSetType(struct Gpio *p, enum gpioType type)
{
  uint32_t *iocon = calcReg(p->pin);

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
