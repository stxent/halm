/*
 * gpio.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "macro.h"
#include "platform/gpio.h"
#include "threading/mutex.h"
#include "platform/nxp/device_defs.h"
#include "platform/nxp/lpc13xx/power.h"
/*----------------------------------------------------------------------------*/
/* Reserved bits, digital mode and IO mode for I2C pins */
#define IOCON_DEFAULT                   0x01D0

#define IOCON_FUNC(func)                BIT_FIELD((func), 0)
#define IOCON_FUNC_MASK                 BIT_FIELD(0x07, 0)

#define IOCON_I2C_STANDARD              BIT_FIELD(0, 8)
#define IOCON_I2C_IO                    BIT_FIELD(1, 8)
#define IOCON_I2C_PLUS                  BIT_FIELD(2, 8)
#define IOCON_I2C_MASK                  BIT_FIELD(0x03, 8)

#define IOCON_MODE_DIGITAL              BIT(7)
#define IOCON_MODE_INACTIVE             BIT_FIELD(0, 3)
#define IOCON_MODE_PULLDOWN             BIT_FIELD(1, 3)
#define IOCON_MODE_PULLUP               BIT_FIELD(2, 3)
#define IOCON_MODE_MASK                 BIT_FIELD(0x03, 3)

#define IOCON_HYS                       BIT(5)
#define IOCON_OD                        BIT(10)
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_TypeDef *calcPort(union GpioPin);
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
static inline LPC_GPIO_TypeDef *calcPort(union GpioPin p)
{
  return (LPC_GPIO_TypeDef *)((void *)LPC_GPIO0 +
      ((void *)LPC_GPIO1 - (void *)LPC_GPIO0) * p.port);
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

  iocon = (void *)LPC_IOCON + gpioRegMap[p.pin.port][p.pin.offset];
  /* PIO function, no pull, no hysteresis, standard output */
  *iocon = IOCON_DEFAULT & ~IOCON_MODE_MASK;

  /* Exceptions */
  if ((p.pin.port == 1 && (p.pin.offset >= 0 && p.pin.offset <= 2))
      || (p.pin.port == 0 && p.pin.offset == 11))
  {
    *iocon |= 0x01; /* Select GPIO function */
  }
  /* TODO Add analog functions */

  if (dir == GPIO_OUTPUT)
    ((LPC_GPIO_TypeDef *)p.reg)->DIR |= 1 << p.pin.offset;
  else
    ((LPC_GPIO_TypeDef *)p.reg)->DIR &= ~(1 << p.pin.offset);

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
  uint32_t *iocon = (void *)LPC_IOCON + gpioRegMap[p->pin.port][p->pin.offset];

  ((LPC_GPIO_TypeDef *)p->reg)->DIR &= ~(1 << p->pin.offset);
  *iocon = IOCON_DEFAULT;

  if (!--instances)
  {
    /* Disable AHB clock to the GPIO domain */
    sysClockDisable(CLK_GPIO);
    /* Enable clock to IO configuration block */
    sysClockDisable(CLK_IOCON);
  }
}
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *p)
{
  return (((LPC_GPIO_TypeDef *)p->reg)->DATA & (1 << p->pin.offset)) != 0;
}
/*----------------------------------------------------------------------------*/
void gpioWrite(struct Gpio *p, uint8_t value)
{
  *(uint32_t *)(((LPC_GPIO_TypeDef *)p->reg)->MASKED_ACCESS
      + (1 << p->pin.offset)) = value ? 0xFFF : 0x000;
}
/*----------------------------------------------------------------------------*/
void gpioSetFunction(struct Gpio *p, uint8_t function)
{
  uint32_t *iocon = (void *)LPC_IOCON + gpioRegMap[p->pin.port][p->pin.offset];

  *iocon &= ~IOCON_FUNC_MASK;
  *iocon |= IOCON_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void gpioSetPull(struct Gpio *p, enum gpioPull pull)
{
  uint32_t *iocon = (void *)LPC_IOCON + gpioRegMap[p->pin.port][p->pin.offset];

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
  uint32_t *iocon = (void *)LPC_IOCON + gpioRegMap[p->pin.port][p->pin.offset];

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
/*----------------------------------------------------------------------------*/
/* Returns zero when pin not initialized */
gpioKey gpioGetKey(struct Gpio *p)
{
  return ~p->pin.key; /* External pin identifiers are in 1's complement form */
}
