/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <entity.h>
#include <pin.h>
#include <platform/nxp/lpc43xx/pin_defs.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define PACK_VALUE(channel, offset)  (((offset) << 3) | (channel))
#define UNPACK_CHANNEL(value)        ((value) & 0x07)
#define UNPACK_OFFSET(value)         (((value) >> 3) & 0x1F)
/*----------------------------------------------------------------------------*/
enum
{
  GPIO_FUNCTION_0 = 0,
  GPIO_FUNCTION_4
};
/*----------------------------------------------------------------------------*/
struct PinHandler
{
  struct Entity parent;

  /* Initialized pins count */
  uint16_t instances;
};
/*----------------------------------------------------------------------------*/
static void *calcControlReg(union PinData);
static void commonPinSetup(struct Pin);
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach();
static inline void pinHandlerDetach();
static enum result pinHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass handlerTable = {
    .size = sizeof(struct PinHandler),
    .init = pinHandlerInit,
    .deinit = 0
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry gpioPins[] = {
    {
        /* From GPIO0_0 to GPIO0_1 */
        .begin = PIN(0, 0),
        .end = PIN(0, 1),
        .channel = 0,
        .value = PACK_VALUE(0, 0)
    }, {
        /* GPIO0_4 */
        .begin = PIN(1, 0),
        .end = PIN(1, 0),
        .channel = 0,
        .value = PACK_VALUE(0, 4)
    }, {
        /* From GPIO0_8 to GPIO0_11 */
        .begin = PIN(1, 1),
        .end = PIN(1, 4),
        .channel = 0,
        .value = PACK_VALUE(0, 8)
    }, {
        /* From GPIO1_8 to GPIO1_9 */
        .begin = PIN(1, 5),
        .end = PIN(1, 6),
        .channel = 0,
        .value = PACK_VALUE(1, 8)
    }, {
        /* From GPIO1_0 to GPIO1_7 */
        .begin = PIN(1, 7),
        .end = PIN(1, 14),
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        /* From GPIO0_2 to GPIO0_3 */
        .begin = PIN(1, 15),
        .end = PIN(1, 16),
        .channel = 0,
        .value = PACK_VALUE(0, 2)
    }, {
        /* From GPIO0_12 to GPIO0_13 */
        .begin = PIN(1, 17),
        .end = PIN(1, 18),
        .channel = 0,
        .value = PACK_VALUE(0, 12)
    }, {
        /* GPIO0_15 */
        .begin = PIN(1, 20),
        .end = PIN(1, 20),
        .channel = 0,
        .value = PACK_VALUE(0, 15)
    }, {
        /* From GPIO5_0 to GPIO5_6 */
        .begin = PIN(2, 0),
        .end = PIN(2, 6),
        .channel = 0,
        .value = PACK_VALUE(5, 0)
    }, {
        /* GPIO0_7 */
        .begin = PIN(2, 7),
        .end = PIN(2, 7),
        .channel = 0,
        .value = PACK_VALUE(0, 7)
    }, {
        /* GPIO5_7 */
        .begin = PIN(2, 8),
        .end = PIN(2, 8),
        .channel = 0,
        .value = PACK_VALUE(5, 7)
    }, {
        /* GPIO1_10 */
        .begin = PIN(2, 9),
        .end = PIN(2, 9),
        .channel = 0,
        .value = PACK_VALUE(1, 10)
    }, {
        /* GPIO0_14 */
        .begin = PIN(2, 10),
        .end = PIN(2, 10),
        .channel = 0,
        .value = PACK_VALUE(0, 14)
    }, {
        /* From GPIO1_11 to GPIO1_13 */
        .begin = PIN(2, 11),
        .end = PIN(2, 13),
        .channel = 0,
        .value = PACK_VALUE(1, 11)
    }, {
        /* From GPIO5_8 to GPIO5_9 */
        .begin = PIN(3, 1),
        .end = PIN(3, 2),
        .channel = 0,
        .value = PACK_VALUE(5, 8)
    }, {
        /* From GPIO1_14 to GPIO1_15 */
        .begin = PIN(3, 4),
        .end = PIN(3, 5),
        .channel = 0,
        .value = PACK_VALUE(1, 14)
    }, {
        /* GPIO0_6 */
        .begin = PIN(3, 6),
        .end = PIN(3, 6),
        .channel = 0,
        .value = PACK_VALUE(0, 6)
    }, {
        /* From GPIO5_10 to GPIO5_11 */
        .begin = PIN(3, 7),
        .end = PIN(3, 8),
        .channel = 0,
        .value = PACK_VALUE(5, 10)
    }, {
        /* From GPIO2_0 to GPIO2_6 */
        .begin = PIN(4, 0),
        .end = PIN(4, 6),
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        /* From GPIO2_9 to GPIO2_15 */
        .begin = PIN(5, 0),
        .end = PIN(5, 6),
        .channel = 0,
        .value = PACK_VALUE(2, 9)
    }, {
        /* From GPIO5_12 to GPIO5_14 */
        .begin = PIN(4, 8),
        .end = PIN(4, 10),
        .channel = 0,
        .value = PACK_VALUE(5, 12)
    }, {
        /* GPIO2_7 */
        .begin = PIN(5, 7),
        .end = PIN(5, 7),
        .channel = 0,
        .value = PACK_VALUE(2, 7)
    }, {
        /* From GPIO3_0 to GPIO3_4 */
        .begin = PIN(6, 1),
        .end = PIN(6, 5),
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        /* GPIO0_5 */
        .begin = PIN(6, 6),
        .end = PIN(6, 6),
        .channel = 0,
        .value = PACK_VALUE(0, 5)
    }, {
        /* From GPIO5_15 to GPIO5_16 */
        .begin = PIN(6, 7),
        .end = PIN(6, 8),
        .channel = 0,
        .value = PACK_VALUE(5, 15)
    }, {
        /* From GPIO3_5 to GPIO3_7 */
        .begin = PIN(6, 9),
        .end = PIN(6, 11),
        .channel = 0,
        .value = PACK_VALUE(3, 5)
    }, {
        /* GPIO2_8 */
        .begin = PIN(6, 12),
        .end = PIN(6, 12),
        .channel = 0,
        .value = PACK_VALUE(2, 8)
    }, {
        /* From GPIO3_8 to GPIO3_15 */
        .begin = PIN(7, 0),
        .end = PIN(7, 7),
        .channel = 0,
        .value = PACK_VALUE(3, 8)
    }, {
        /* From GPIO4_0 to GPIO4_7 */
        .begin = PIN(8, 0),
        .end = PIN(8, 7),
        .channel = 0,
        .value = PACK_VALUE(4, 0)
    }, {
        /* From GPIO4_12 to GPIO4_15 */
        .begin = PIN(9, 0),
        .end = PIN(9, 3),
        .channel = 0,
        .value = PACK_VALUE(4, 12)
    }, {
        /* From GPIO5_17 to GPIO5_18 */
        .begin = PIN(9, 4),
        .end = PIN(9, 5),
        .channel = 0,
        .value = PACK_VALUE(5, 17)
    }, {
        /* GPIO4_11 */
        .begin = PIN(9, 6),
        .end = PIN(9, 6),
        .channel = 0,
        .value = PACK_VALUE(4, 11)
    }, {
        /* From GPIO4_8 to GPIO4_10 */
        .begin = PIN(0xA, 1),
        .end = PIN(0xA, 3),
        .channel = 0,
        .value = PACK_VALUE(4, 8)
    }, {
        /* GPIO5_19 */
        .begin = PIN(0xA, 4),
        .end = PIN(0xA, 4),
        .channel = 0,
        .value = PACK_VALUE(5, 19)
    }, {
        /* From GPIO5_20 to GPIO5_26 */
        .begin = PIN(0xB, 0),
        .end = PIN(0xB, 6),
        .channel = 0,
        .value = PACK_VALUE(5, 20)
    }, {
        /* From GPIO6_0 to GPIO6_13 */
        .begin = PIN(0xC, 1),
        .end = PIN(0xC, 14),
        .channel = 0,
        .value = PACK_VALUE(6, 0)
    }, {
        /* From GPIO6_14 to GPIO6_30 */
        .begin = PIN(0xD, 0),
        .end = PIN(0xD, 16),
        .channel = 0,
        .value = PACK_VALUE(6, 14)
    }, {
        /* From GPIO7_0 to GPIO7_15 */
        .begin = PIN(0xE, 0),
        .end = PIN(0xE, 15),
        .channel = 0,
        .value = PACK_VALUE(7, 0)
    }, {
        /* From GPIO7_16 to GPIO7_18 */
        .begin = PIN(0xF, 1),
        .end = PIN(0xF, 3),
        .channel = 0,
        .value = PACK_VALUE(7, 16)
    }, {
        /* From GPIO7_19 to GPIO7_25 */
        .begin = PIN(0xF, 5),
        .end = PIN(0xF, 11),
        .channel = 0,
        .value = PACK_VALUE(7, 19)
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const PinHandler = &handlerTable;
static struct PinHandler *pinHandler = 0;
/*----------------------------------------------------------------------------*/
static void *calcControlReg(union PinData data)
{
  return (void *)(sizeof(LPC_SCU->SFSP0[0]) * data.offset
      + ((uint32_t)&LPC_SCU->SFSP1 - (uint32_t)&LPC_SCU->SFSP0) * data.port
      + (uint32_t)&LPC_SCU->SFSP0);
}
/*----------------------------------------------------------------------------*/
static void commonPinSetup(struct Pin pin)
{
  /* Register new pin in the handler */
  pinHandlerAttach();

  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach()
{
  /* Create handler object on first function call */
  if (!pinHandler)
    pinHandler = init(PinHandler, 0);

  if (!pinHandler->instances++)
  {
    //TODO Enable clocks
  }
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerDetach()
{
  /* Disable clocks when no active pins exist */
  if (!--pinHandler->instances)
  {
    //TODO Disable clocks
  }
}
/*----------------------------------------------------------------------------*/
static enum result pinHandlerInit(void *object,
    const void *configPtr __attribute__((unused)))
{
  struct PinHandler * const handler = object;

  handler->instances = 0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(pin_t id)
{
  const struct PinGroupEntry *group;
  struct Pin pin;

  //TODO High-drive pins
  group = pinGroupFind(gpioPins, id, 0);

  if (group)
  {
    union PinData begin, current;

    begin.key = ~group->begin;
    current.key = ~id;

    pin.data.port = UNPACK_CHANNEL(group->value);
    pin.data.offset = current.offset - begin.offset
        + UNPACK_OFFSET(group->value);
    pin.reg = calcControlReg(current);
  }
  else
  {
    pin.data.key = ~0;
    pin.reg = 0;
  }

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinSetup(pin);
  LPC_GPIO->DIR[pin.data.port] &= ~(1 << pin.data.offset);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinSetup(pin);
  LPC_GPIO->DIR[pin.data.port] |= 1 << pin.data.offset;
  pinWrite(pin, value);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  volatile uint32_t * const reg = pin.reg;
  const uint32_t value = *reg;

  switch (function)
  {
    case PIN_DEFAULT:
      function = pin.data.port >= 5 ? 4 : 0;
      break;

    case PIN_ANALOG:
      //TODO
      return;
  }

  *reg = (value & ~SFS_FUNC_MASK) | SFS_FUNC(function);
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
  volatile uint32_t * const reg = pin.reg;
  uint32_t value = *reg & ~SFS_PULL_MASK;

  switch (pull)
  {
    case PIN_NOPULL:
      value |= SFS_PULL_INACTIVE;
      break;

    case PIN_PULLUP:
      value |= SFS_PULL_PULLUP;
      break;

    case PIN_PULLDOWN:
      value |= SFS_PULL_PULLDOWN;
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum pinSlewRate rate)
{
  //TODO Different setup for normal, high-drive and high-speed pins
}
