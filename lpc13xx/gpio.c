/*
 * gpio.c
 *
 *  Created on: Sep 8, 2012
 *      Author: xen
 */

#include <stdlib.h>
#include "gpio.h"
#include "lpc13xx_defines.h"
/*----------------------------------------------------------------------------*/
/* IOCON register map differs for LPC1315/16/17/45/46/47 */
#if defined __LPC13XX
/*----------------------------------------------------------------------------*/
#define IOCON_FUNC(func)                ((func) << 0)
#define IOCON_FUNC_MASK                 IOCON_FUNC(0x07)

#define IOCON_MODE_INACTIVE             (0 << 3)
#define IOCON_MODE_PULLDOWN             (1 << 3)
#define IOCON_MODE_PULLUP               (2 << 3)
#define IOCON_MODE_MASK                 (0x03 << 3)

#define IOCON_HYS                       BIT(5)
#define IOCON_OD                        BIT(10)
/*----------------------------------------------------------------------------*/
static const uint8_t gpioRegMap[4][12] = {
    {0x0C, 0x10, 0x1C, 0x2C, 0x30, 0x34, 0x4C, 0x50, 0x60, 0x64, 0x68, 0x74},
    {0x78, 0x7C, 0x80, 0x90, 0x94, 0xA0, 0xA4, 0xA8, 0x14, 0x38, 0x6C, 0x98},
    {0x08, 0x28, 0x5C, 0x8C, 0x40, 0x44, 0x00, 0x20, 0x24, 0x54, 0x58, 0x70},
    {0x84, 0x88, 0x9C, 0xAC, 0x3C, 0x48}
};
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
//static const char *strToInt(const char *, int *);
//static union GpioPin nameToPin(const char *);
/*----------------------------------------------------------------------------*/
//static const char *strToInt(const char *str, int *result)
//{
//  int res = 0;
//
////  while ((*str >= '0') && (*str <= '9'))
//  while ((uint8_t)(*str - '0') <= '9')
//  {
//    res *= 10;
//    res += *str++ - '0';
//  }
//  *result = res;
//
//  return str;
//}
/*----------------------------------------------------------------------------*/
///* Name format is "PX.Y" where X is port and Y is pin */
///* Return combined port and pin numbers or -1 on error */
//static union GpioPin nameToPin(const char *name)
//{
//  int num;
//  union GpioPin result = {
//      .id = -1
//  };
//
//  if (*name++ != 'P')
//    return result;
//  name = strToInt(name, &num);
//  result.port = (int8_t)num;
//
//  if (*name++ != '.')
//    return result;
//  name = strToInt(name, &num);
//  result.offset = (int8_t)num;
//
//  return result;
//}
/*----------------------------------------------------------------------------*/
void gpioEnable()
{
  /* Enable AHB clock to the GPIO domain */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 6); //FIXME

#ifdef __JTAG_DISABLED
  LPC_IOCON->JTAG_TDO_PIO1_1  &= ~0x07;
  LPC_IOCON->JTAG_TDO_PIO1_1  |= 0x01;
#endif

//  /* Set up NVIC when I/O pins are configured as external interrupts. */
//  NVIC_EnableIRQ(EINT0_IRQn);
//  NVIC_EnableIRQ(EINT1_IRQn);
//  NVIC_EnableIRQ(EINT2_IRQn);
//  NVIC_EnableIRQ(EINT3_IRQn);
}
/*----------------------------------------------------------------------------*/
void gpioDisable()
{
  LPC_SYSCON->SYSAHBCLKCTRL &= ~(1 << 6); //FIXME
}
/*----------------------------------------------------------------------------*/
//struct Gpio gpioInitByName(const char *name, enum gpioDir dir)
//{
//  uint32_t *iocon;
//  union GpioPin converted = {
//      .id = -1
//  };
//  struct Gpio p = {
//      .control = 0,
//      .pin     = converted
//  };
//
//  converted = nameToPin(name);
//  switch (converted.port)
//  {
//    case 0:
//      p.control = LPC_GPIO0;
//      break;
//    case 1:
//      p.control = LPC_GPIO1;
//      break;
//    case 2:
//      p.control = LPC_GPIO2;
//      break;
//    case 3:
//      p.control = LPC_GPIO3;
//      break;
//    default:
//      return p;
//  }
//  p.pin = converted;
//
//  iocon = (void *)LPC_IOCON + gpioRegMap[p.pin.port][p.pin.offset];
//  /* PIO function, no pull, no hysteresis, standard output */
//  *iocon = IOCON_FUNC(0) | IOCON_MODE_INACTIVE;
//
//  if (dir == GPIO_OUTPUT)
//    p.control->DIR |= 1 << p.pin.offset;
//  else
//    p.control->DIR &= ~(1 << p.pin.offset);
//
//  return p;
//}
/*----------------------------------------------------------------------------*/
/* Return 0 when no function associated with id found */
uint8_t gpioFindMode(const struct GpioPinMode *pinList, int16_t id)
{
  while (pinList->id != -1)
  {
    if (pinList->id == id)
      return pinList->mode;
    pinList++;
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
struct Gpio gpioInitPin(int16_t id, enum gpioDir dir)
{
  uint32_t *iocon;
  union GpioPin converted = {
      .id = -1
  };
  struct Gpio p = {
      .control = 0,
      .pin     = converted
  };

  converted.id = id;
  switch (converted.port)
  {
    case 0:
      p.control = LPC_GPIO0;
      break;
    case 1:
      p.control = LPC_GPIO1;
      break;
    case 2:
      p.control = LPC_GPIO2;
      break;
    case 3:
      p.control = LPC_GPIO3;
      break;
    default:
      return p;
  }
  p.pin = converted;

  iocon = (void *)LPC_IOCON + gpioRegMap[p.pin.port][p.pin.offset];
  /* PIO function, no pull, no hysteresis, standard output */
  *iocon = IOCON_FUNC(0) | IOCON_MODE_INACTIVE;

  if (dir == GPIO_OUTPUT)
    p.control->DIR |= 1 << p.pin.offset;
  else
    p.control->DIR &= ~(1 << p.pin.offset);

  return p;
}
/*----------------------------------------------------------------------------*/
void gpioReleasePin(struct Gpio *p)
{
  uint32_t *iocon = (void *)LPC_IOCON + gpioRegMap[p->pin.port][p->pin.offset];
  p->control->DIR &= ~(1 << p->pin.offset);
  *iocon = 0;
}
/*----------------------------------------------------------------------------*/
uint8_t gpioRead(struct Gpio *p)
{
  return ((p->control->DATA & (1 << p->pin.offset)) != 0) ? 1 : 0;
}
/*----------------------------------------------------------------------------*/
void gpioWrite(struct Gpio *p, uint8_t value)
{
  if (value)
    p->control->DATA |= 1 << p->pin.offset;
  else
    p->control->DATA &= ~(1 << p->pin.offset);
}
/*----------------------------------------------------------------------------*/
void gpioSetMode(struct Gpio *p, uint8_t mode)
{
  uint32_t *iocon = (void *)LPC_IOCON + gpioRegMap[p->pin.port][p->pin.offset];
  *iocon &= ~IOCON_FUNC_MASK;
  *iocon |= IOCON_FUNC(mode);
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
int16_t gpioGetId(struct Gpio *p)
{
  return p->pin.id;
}
