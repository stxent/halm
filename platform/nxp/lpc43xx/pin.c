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
static void commonPinSetup(struct Pin);
static enum pinDriveType detectPinDriveType(volatile uint32_t *);
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
const struct PinEntry analogPins[] = {
    {
        .key = PIN(PORT_4, 3), /* ADC0_0 */
        .channel = 0,
        .value = PACK_VALUE(0, 0)
    }, {
        .key = PIN(PORT_4, 1), /* ADC0_1 */
        .channel = 0,
        .value = PACK_VALUE(0, 1)
    }, {
        .key = PIN(PORT_F, 8), /* ADC0_2 */
        .channel = 0,
        .value = PACK_VALUE(0, 2)
    }, {
        .key = PIN(PORT_7, 5), /* ADC0_3 */
        .channel = 0,
        .value = PACK_VALUE(0, 3)
    }, {
        .key = PIN(PORT_7, 4), /* ADC0_4 */
        .channel = 0,
        .value = PACK_VALUE(0, 4)
    }, {
        .key = PIN(PORT_F, 10), /* ADC0_5 */
        .channel = 0,
        .value = PACK_VALUE(0, 5)
    }, {
        .key = PIN(PORT_B, 6), /* ADC0_6 */
        .channel = 0,
        .value = PACK_VALUE(0, 6)
    }, {
        .key = PIN(PORT_C, 3), /* ADC1_0 */
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(PORT_C, 0), /* ADC1_1 */
        .channel = 0,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_F, 9), /* ADC1_2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(PORT_F, 6), /* ADC1_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_F, 5), /* ADC1_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_F, 11), /* ADC1_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_7, 7), /* ADC1_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_F, 7), /* ADC1_7 */
        .channel = 0,
        .value = PACK_VALUE(1, 7)
    }, {
        .key = PIN(PORT_4, 4), /* DAC */
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = 0 /* End of pin function association list */
    }
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
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcControlReg(union PinData data)
{
  if (data.port != PORT_CLK)
  {
    const uint32_t portMapSize = ((uint32_t)&LPC_SCU->SFSP1
        - (uint32_t)&LPC_SCU->SFSP0) / sizeof(LPC_SCU->SFSP0[0]);

    return LPC_SCU->SFSP0 + portMapSize * data.port + data.offset;
  }
  else
  {
    return &LPC_SCU->SFSPCLK0 + data.offset;
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
    const uint32_t totalOffset = reg - &LPC_SCU->SFSPCLK0;

    pin.port = totalOffset / portMapSize;
    pin.offset = totalOffset % portMapSize;
  }

  return pin;
}
/*----------------------------------------------------------------------------*/
static void commonPinSetup(struct Pin pin)
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
static inline void pinHandlerAttach()
{
  /* Create handler object on first function call */
  if (!pinHandler)
    pinHandler = init(PinHandler, 0);

  if (!pinHandler->instances++)
  {
    sysClockEnable(CLK_M4_GPIO);

    sysResetEnable(RST_SCU);
    sysResetEnable(RST_GPIO);
  }
}
/*----------------------------------------------------------------------------*/
static inline void pinHandlerDetach()
{
  /* Disable clocks when no active pins exist */
  if (!--pinHandler->instances)
  {
    sysClockDisable(CLK_M4_GPIO);
  }
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

  group = pinGroupFind(gpioPins, id, 0);

  if (group)
  {
    union PinData begin, current;

    begin.key = ~group->begin;
    current.key = ~id;

    pin.data.port = UNPACK_CHANNEL(group->value);
    pin.data.offset = current.offset - begin.offset
        + UNPACK_OFFSET(group->value);
    pin.reg = (void *)calcControlReg(current);
  }
  else
  {
    /* Some pins do not have GPIO function */
    pin.reg = (void *)calcControlReg(pin.data);
    pin.data.key = ~0;
  }

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  commonPinSetup(pin);

  if ((pin_t)(pin.data.key))
  {
    /* Configure pin as input */
    LPC_GPIO->DIR[pin.data.port] &= ~(1 << pin.data.offset);
  }

  volatile uint32_t * const reg = pin.reg;

  /* Disable glitch filter and enable input buffer */
  *reg |= SFS_ZIF | SFS_EZI;
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, uint8_t value)
{
  commonPinSetup(pin);

  if ((pin_t)(pin.data.key))
  {
    /* Configure pin as output */
    LPC_GPIO->DIR[pin.data.port] |= 1 << pin.data.offset;
    /* Set default output value */
    pinWrite(pin, value);
  }

  volatile uint32_t * const reg = pin.reg;

  /* Enable glitch filter and disable input buffer */
  *reg &= ~(SFS_ZIF | SFS_EZI);
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  volatile uint32_t * const reg = pin.reg;
  const union PinData data = calcPinData(reg);
  const uint32_t value = *reg & ~SFS_FUNC_MASK;
  const struct PinEntry *pinEntry = pinFind(analogPins, data.key, 0);

  switch (function)
  {
    case PIN_DEFAULT:
      if (!(pin_t)(pin.data.key))
        break;

      function = pin.data.port >= 5 ? 4 : 0;

      if (pinEntry)
      {
        *(&LPC_SCU->ENAIO0 + UNPACK_CHANNEL(pinEntry->value)) &=
            ~(1 << UNPACK_OFFSET(pinEntry->value));
      }
      *reg = value | SFS_FUNC(function);
      break;

    case PIN_ANALOG:
      if (pinEntry)
      {
        /*
         * Pin should be completely reconfigured to switch back
         * to digital function from analog function.
         */
        *reg = value & ~SFS_EZI;

        *(&LPC_SCU->ENAIO0 + UNPACK_CHANNEL(pinEntry->value)) |=
            1 << UNPACK_OFFSET(pinEntry->value);
      }
      break;

    default:
      *reg = value | SFS_FUNC(function);
      break;
  }
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum pinPull pull)
{
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
  volatile uint32_t * const reg = pin.reg;
  const enum pinDriveType type = detectPinDriveType(reg);
  uint32_t value = *reg & ~SFS_STRENGTH_MASK;

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
        value |= SFS_STRENGTH_ULTRAHIGH;
        break;
    }
  }
  else
  {
    if (rate == PIN_SLEW_FAST)
      value |= SFS_EHS;
    else
      value &= ~SFS_EHS;
  }

  *reg = value;
}
