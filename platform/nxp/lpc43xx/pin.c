/*
 * pin.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <entity.h>
#include <pin.h>
#include <spinlock.h>
#include <platform/nxp/lpc43xx/pin_defs.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define PACK_VALUE(channel, offset) (((offset) << 3) | (channel))
#define UNPACK_CHANNEL(value)       ((value) & 0x07)
#define UNPACK_OFFSET(value)        (((value) >> 3) & 0x1F)
/*----------------------------------------------------------------------------*/
enum pinDriveType
{
  NORMAL_DRIVE_PIN,
  HIGH_DRIVE_PIN,
  HIGH_SPEED_PIN
};
/*----------------------------------------------------------------------------*/
struct PinHandler
{
  struct Entity parent;

  /* Initialized pins count */
  uint16_t instances;
};
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(union PinData);
static union PinData calcPinData(volatile uint32_t *);
static void commonPinInit(struct Pin);
static enum pinDriveType detectPinDriveType(volatile uint32_t *);
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach(void);
static inline void pinHandlerDetach(void);
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
        .begin = PIN(PORT_0, 0),
        .end = PIN(PORT_0, 1),
        .channel = 0,
        .value = PACK_VALUE(0, 0)
    }, {
        /* GPIO0_4 */
        .begin = PIN(PORT_1, 0),
        .end = PIN(PORT_1, 0),
        .channel = 0,
        .value = PACK_VALUE(0, 4)
    }, {
        /* From GPIO0_8 to GPIO0_11 */
        .begin = PIN(PORT_1, 1),
        .end = PIN(PORT_1, 4),
        .channel = 0,
        .value = PACK_VALUE(0, 8)
    }, {
        /* From GPIO1_8 to GPIO1_9 */
        .begin = PIN(PORT_1, 5),
        .end = PIN(PORT_1, 6),
        .channel = 0,
        .value = PACK_VALUE(1, 8)
    }, {
        /* From GPIO1_0 to GPIO1_7 */
        .begin = PIN(PORT_1, 7),
        .end = PIN(PORT_1, 14),
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        /* From GPIO0_2 to GPIO0_3 */
        .begin = PIN(PORT_1, 15),
        .end = PIN(PORT_1, 16),
        .channel = 0,
        .value = PACK_VALUE(0, 2)
    }, {
        /* From GPIO0_12 to GPIO0_13 */
        .begin = PIN(PORT_1, 17),
        .end = PIN(PORT_1, 18),
        .channel = 0,
        .value = PACK_VALUE(0, 12)
    }, {
        /* GPIO0_15 */
        .begin = PIN(PORT_1, 20),
        .end = PIN(PORT_1, 20),
        .channel = 0,
        .value = PACK_VALUE(0, 15)
    }, {
        /* From GPIO5_0 to GPIO5_6 */
        .begin = PIN(PORT_2, 0),
        .end = PIN(PORT_2, 6),
        .channel = 0,
        .value = PACK_VALUE(5, 0)
    }, {
        /* GPIO0_7 */
        .begin = PIN(PORT_2, 7),
        .end = PIN(PORT_2, 7),
        .channel = 0,
        .value = PACK_VALUE(0, 7)
    }, {
        /* GPIO5_7 */
        .begin = PIN(PORT_2, 8),
        .end = PIN(PORT_2, 8),
        .channel = 0,
        .value = PACK_VALUE(5, 7)
    }, {
        /* GPIO1_10 */
        .begin = PIN(PORT_2, 9),
        .end = PIN(PORT_2, 9),
        .channel = 0,
        .value = PACK_VALUE(1, 10)
    }, {
        /* GPIO0_14 */
        .begin = PIN(PORT_2, 10),
        .end = PIN(PORT_2, 10),
        .channel = 0,
        .value = PACK_VALUE(0, 14)
    }, {
        /* From GPIO1_11 to GPIO1_13 */
        .begin = PIN(PORT_2, 11),
        .end = PIN(PORT_2, 13),
        .channel = 0,
        .value = PACK_VALUE(1, 11)
    }, {
        /* From GPIO5_8 to GPIO5_9 */
        .begin = PIN(PORT_3, 1),
        .end = PIN(PORT_3, 2),
        .channel = 0,
        .value = PACK_VALUE(5, 8)
    }, {
        /* From GPIO1_14 to GPIO1_15 */
        .begin = PIN(PORT_3, 4),
        .end = PIN(PORT_3, 5),
        .channel = 0,
        .value = PACK_VALUE(1, 14)
    }, {
        /* GPIO0_6 */
        .begin = PIN(PORT_3, 6),
        .end = PIN(PORT_3, 6),
        .channel = 0,
        .value = PACK_VALUE(0, 6)
    }, {
        /* From GPIO5_10 to GPIO5_11 */
        .begin = PIN(PORT_3, 7),
        .end = PIN(PORT_3, 8),
        .channel = 0,
        .value = PACK_VALUE(5, 10)
    }, {
        /* From GPIO2_0 to GPIO2_6 */
        .begin = PIN(PORT_4, 0),
        .end = PIN(PORT_4, 6),
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        /* From GPIO5_12 to GPIO5_14 */
        .begin = PIN(PORT_4, 8),
        .end = PIN(PORT_4, 10),
        .channel = 0,
        .value = PACK_VALUE(5, 12)
    }, {
        /* From GPIO2_9 to GPIO2_15 */
        .begin = PIN(PORT_5, 0),
        .end = PIN(PORT_5, 6),
        .channel = 0,
        .value = PACK_VALUE(2, 9)
    }, {
        /* GPIO2_7 */
        .begin = PIN(PORT_5, 7),
        .end = PIN(PORT_5, 7),
        .channel = 0,
        .value = PACK_VALUE(2, 7)
    }, {
        /* From GPIO3_0 to GPIO3_4 */
        .begin = PIN(PORT_6, 1),
        .end = PIN(PORT_6, 5),
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        /* GPIO0_5 */
        .begin = PIN(PORT_6, 6),
        .end = PIN(PORT_6, 6),
        .channel = 0,
        .value = PACK_VALUE(0, 5)
    }, {
        /* From GPIO5_15 to GPIO5_16 */
        .begin = PIN(PORT_6, 7),
        .end = PIN(PORT_6, 8),
        .channel = 0,
        .value = PACK_VALUE(5, 15)
    }, {
        /* From GPIO3_5 to GPIO3_7 */
        .begin = PIN(PORT_6, 9),
        .end = PIN(PORT_6, 11),
        .channel = 0,
        .value = PACK_VALUE(3, 5)
    }, {
        /* GPIO2_8 */
        .begin = PIN(PORT_6, 12),
        .end = PIN(PORT_6, 12),
        .channel = 0,
        .value = PACK_VALUE(2, 8)
    }, {
        /* From GPIO3_8 to GPIO3_15 */
        .begin = PIN(PORT_7, 0),
        .end = PIN(PORT_7, 7),
        .channel = 0,
        .value = PACK_VALUE(3, 8)
    }, {
        /* From GPIO4_0 to GPIO4_7 */
        .begin = PIN(PORT_8, 0),
        .end = PIN(PORT_8, 7),
        .channel = 0,
        .value = PACK_VALUE(4, 0)
    }, {
        /* From GPIO4_12 to GPIO4_15 */
        .begin = PIN(PORT_9, 0),
        .end = PIN(PORT_9, 3),
        .channel = 0,
        .value = PACK_VALUE(4, 12)
    }, {
        /* From GPIO5_17 to GPIO5_18 */
        .begin = PIN(PORT_9, 4),
        .end = PIN(PORT_9, 5),
        .channel = 0,
        .value = PACK_VALUE(5, 17)
    }, {
        /* GPIO4_11 */
        .begin = PIN(PORT_9, 6),
        .end = PIN(PORT_9, 6),
        .channel = 0,
        .value = PACK_VALUE(4, 11)
    }, {
        /* From GPIO4_8 to GPIO4_10 */
        .begin = PIN(PORT_A, 1),
        .end = PIN(PORT_A, 3),
        .channel = 0,
        .value = PACK_VALUE(4, 8)
    }, {
        /* GPIO5_19 */
        .begin = PIN(PORT_A, 4),
        .end = PIN(PORT_A, 4),
        .channel = 0,
        .value = PACK_VALUE(5, 19)
    }, {
        /* From GPIO5_20 to GPIO5_26 */
        .begin = PIN(PORT_B, 0),
        .end = PIN(PORT_B, 6),
        .channel = 0,
        .value = PACK_VALUE(5, 20)
    }, {
        /* From GPIO6_0 to GPIO6_13 */
        .begin = PIN(PORT_C, 1),
        .end = PIN(PORT_C, 14),
        .channel = 0,
        .value = PACK_VALUE(6, 0)
    }, {
        /* From GPIO6_14 to GPIO6_30 */
        .begin = PIN(PORT_D, 0),
        .end = PIN(PORT_D, 16),
        .channel = 0,
        .value = PACK_VALUE(6, 14)
    }, {
        /* From GPIO7_0 to GPIO7_15 */
        .begin = PIN(PORT_E, 0),
        .end = PIN(PORT_E, 15),
        .channel = 0,
        .value = PACK_VALUE(7, 0)
    }, {
        /* From GPIO7_16 to GPIO7_18 */
        .begin = PIN(PORT_F, 1),
        .end = PIN(PORT_F, 3),
        .channel = 0,
        .value = PACK_VALUE(7, 16)
    }, {
        /* From GPIO7_19 to GPIO7_25 */
        .begin = PIN(PORT_F, 5),
        .end = PIN(PORT_F, 11),
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
static spinlock_t spinlock = SPIN_UNLOCKED;
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(union PinData data)
{
  if (data.port < PORT_CLK)
  {
    const uint32_t portMapSize = ((uint32_t)&LPC_SCU->SFSP1
        - (uint32_t)&LPC_SCU->SFSP0) / sizeof(LPC_SCU->SFSP0[0]);

    return LPC_SCU->SFSP0 + portMapSize * data.port + data.offset;
  }
  else if (data.port == PORT_CLK)
  {
    return &LPC_SCU->SFSPCLK0 + data.offset;
  }
  else
  {
    return 0;
  }
}
/*----------------------------------------------------------------------------*/
static union PinData calcPinData(volatile uint32_t *reg)
{
  union PinData pin;

  if (reg >= &LPC_SCU->SFSPCLK0 && reg <= &LPC_SCU->SFSPCLK3)
  {
    pin.port = PORT_CLK;
    pin.offset = reg - &LPC_SCU->SFSPCLK0;
  }
  else
  {
    const uint32_t portMapSize = ((uint32_t)&LPC_SCU->SFSP1
        - (uint32_t)&LPC_SCU->SFSP0) / sizeof(LPC_SCU->SFSP0[0]);
    const uint32_t totalOffset = reg - &LPC_SCU->SFSP0[0];

    pin.port = totalOffset / portMapSize;
    pin.offset = totalOffset % portMapSize;
  }

  pin.key = ~pin.key;
  return pin;
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  /* Register new pin in the handler */
  pinHandlerAttach();

  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetSlewRate(pin, PIN_SLEW_FAST);
}
/*----------------------------------------------------------------------------*/
static enum pinDriveType detectPinDriveType(volatile uint32_t *reg)
{
  const union PinData pin = calcPinData(reg);
  bool highDrive = false;
  bool highSpeed = false;

  highDrive |= pin.port == PORT_1 && pin.offset == 17;
  highDrive |= pin.port == PORT_2 && pin.offset >= 3 && pin.offset <= 5;
  highDrive |= pin.port == PORT_8 && pin.offset <= 2;
  highDrive |= pin.port == PORT_A && pin.offset >= 1 && pin.offset <= 3;

  highSpeed |= pin.port == PORT_3 && pin.offset == 3;
  highSpeed |= pin.port == PORT_CLK && pin.offset <= 3;

  if (highDrive)
    return HIGH_DRIVE_PIN;
  else if (highSpeed)
    return HIGH_SPEED_PIN;
  else
    return NORMAL_DRIVE_PIN;
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerAttach(void)
{
  spinLock(&spinlock);

  /* Create handler object on first function call */
  if (!pinHandler)
    pinHandler = init(PinHandler, 0);
  assert(pinHandler);

  if (!pinHandler->instances++)
  {
    /* CLK_M4_SCU and CLK_M4_GPIO are enabled by default */
    sysResetEnable(RST_SCU);
    sysResetEnable(RST_GPIO);
  }

  spinUnlock(&spinlock);
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerDetach(void)
{
  spinLock(&spinlock);
  --pinHandler->instances;
  spinUnlock(&spinlock);
}
/*----------------------------------------------------------------------------*/
static enum result pinHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
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
  union PinData current;

  current.key = ~id;
  pin.reg = (void *)calcControlReg(current);

  if ((group = pinGroupFind(gpioPins, id, 0)))
  {
    union PinData begin;

    begin.key = ~group->begin;

    pin.data.port = UNPACK_CHANNEL(group->value);
    pin.data.offset = current.offset - begin.offset
        + UNPACK_OFFSET(group->value);
  }
  else
  {
    /* Some pins do not have GPIO function */
    pin.data.key = ~0;

    switch (current.port)
    {
      case PORT_ADC:
        if (current.offset < 8)
          pin.data = current;
        break;

      case PORT_I2C:
        if (current.offset < 2)
        {
          pin.data = current;
          /* Enable input receiver, input filter is enabled by default */
          LPC_SCU->SFSI2C0 |= SFS_I2C_EZI << (current.offset << 3);
        }
        break;

      default:
        break;
    }
  }

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinInit(pin);

  if (pinGpioValid(pin))
  {
    /* Configure pin as input */
    LPC_GPIO->DIR[pin.data.port] &= ~(1 << pin.data.offset);
  }
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinInit(pin);

  if (pinGpioValid(pin))
  {
    /* Configure pin as output */
    LPC_GPIO->DIR[pin.data.port] |= 1 << pin.data.offset;
    /* Set default output value */
    pinWrite(pin, value);
  }
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  if (!pin.reg)
    return;

  volatile uint32_t * const reg = pin.reg;
  uint32_t value = *reg & ~SFS_FUNC_MASK;

  switch (function)
  {
    case PIN_DEFAULT:
      if (pinGpioValid(pin))
      {
        const uint8_t actualFunction = pin.data.port >= 5 ? 4 : 0;

        /* Set GPIO function */
        value |= SFS_FUNC(actualFunction);
      }

      /* Enable input buffer */
      value |= SFS_EZI;
      break;

    case PIN_ANALOG:
      /* Disable input buffer */
      value &= ~SFS_EZI;
      break;

    default:
      value |= SFS_FUNC(function);
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
  if (!pin.reg)
    return;

  volatile uint32_t * const reg = pin.reg;
  uint32_t value = *reg & ~SFS_MODE_MASK;

  switch (pull)
  {
    case PIN_NOPULL:
      value |= SFS_MODE_INACTIVE;
      break;

    case PIN_PULLUP:
      value |= SFS_MODE_PULLUP;
      break;

    case PIN_PULLDOWN:
      value |= SFS_MODE_PULLDOWN;
      break;
  }

  *reg = value;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum pinSlewRate rate)
{
  if (pin.data.port == PORT_I2C)
  {
    const uint32_t mask = (SFS_I2C_EFP | SFS_I2C_EHD) << (pin.data.offset << 3);

    if (rate == PIN_SLEW_FAST)
      LPC_SCU->SFSI2C0 |= mask;
    else
      LPC_SCU->SFSI2C0 &= ~mask;

    return;
  }

  if (!pin.reg)
    return;

  volatile uint32_t * const reg = pin.reg;
  const enum pinDriveType type = detectPinDriveType(reg);
  uint32_t value = *reg & ~(SFS_STRENGTH_MASK | SFS_ZIF);

  if (type == HIGH_SPEED_PIN)
  {
    switch (rate)
    {
      case PIN_SLEW_SLOW:
        value |= SFS_STRENGTH_NORMAL;
        break;

      case PIN_SLEW_NORMAL:
        /* TODO Select between medium and high drive strength */
        value |= SFS_STRENGTH_HIGH;
        break;

      case PIN_SLEW_FAST:
        /* Select drive strength and disable input glitch filter */
        value |= SFS_STRENGTH_ULTRAHIGH | SFS_ZIF;
        break;
    }
  }
  else
  {
    if (rate == PIN_SLEW_FAST)
      value |= SFS_EHS | SFS_ZIF;
    else
      value &= ~SFS_EHS;
  }

  *reg = value;
}
