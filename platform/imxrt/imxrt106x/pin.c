/*
 * pin.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/pin.h>
#include <halm/platform/imxrt/imxrt106x/pin_defs.h>
#include <halm/platform/imxrt/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define SNVS_NONMUX_OFFSET          3
static_assert(PIN_SNVS_TEST_MODE == SNVS_NONMUX_OFFSET,
    "Incorrect GPIO definitions");

#define PACK_VALUE(channel, offset) (((offset) << 3) | (channel))
#define UNPACK_CHANNEL(value)       ((value) & 0x07)
#define UNPACK_OFFSET(value)        (((value) >> 3) & 0x1F)

struct PinDescriptor
{
  uint8_t number;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
static uint16_t calcControlIndex(uint8_t, uint8_t);
static IMX_GPIO_Type *calcPortBase(uint8_t, uint8_t);
static void commonPinInit(struct Pin);
static void enablePortClock(struct Pin);
static volatile uint32_t *getMuxControlReg(struct Pin);
static volatile uint32_t *getPadControlReg(struct Pin);
static volatile uint32_t *getPortRemapReg(struct Pin);
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry gpioPins[] = {
    {
        /* From GPIO1_IO00 to GPIO1_IO15 */
        .begin = PIN(PORT_AD_B0, 0),
        .end = PIN(PORT_AD_B0, 15),
        .channel = 0,
        .value = PACK_VALUE(0, 0)
    }, {
        /* From GPIO1_IO16 to GPIO1_IO31 */
        .begin = PIN(PORT_AD_B1, 0),
        .end = PIN(PORT_AD_B1, 15),
        .channel = 0,
        .value = PACK_VALUE(0, 16)
    }, {
        /* From GPIO2_IO00 to GPIO2_IO15 */
        .begin = PIN(PORT_B0, 0),
        .end = PIN(PORT_B0, 15),
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        /* From GPIO2_IO16 to GPIO2_IO31 */
        .begin = PIN(PORT_B1, 0),
        .end = PIN(PORT_B1, 15),
        .channel = 0,
        .value = PACK_VALUE(1, 16)
    }, {
        /* From GPIO3_IO00 to GPIO3_IO11 */
        .begin = PIN(PORT_SD_B1, 0),
        .end = PIN(PORT_SD_B1, 11),
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        /* From GPIO3_IO12 to GPIO3_IO17 */
        .begin = PIN(PORT_SD_B0, 0),
        .end = PIN(PORT_SD_B0, 5),
        .channel = 0,
        .value = PACK_VALUE(2, 12)
    }, {
        /* From GPIO3_IO18 to GPIO3_IO27 */
        .begin = PIN(PORT_EMC, 32),
        .end = PIN(PORT_EMC, 41),
        .channel = 0,
        .value = PACK_VALUE(2, 18)
    }, {
        /* From GPIO4_IO00 to GPIO4_IO31 */
        .begin = PIN(PORT_EMC, 0),
        .end = PIN(PORT_EMC, 31),
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        /* From GPIO5_IO00 to GPIO5_IO02 */
        .begin = PIN(PORT_SNVS, PIN_SNVS_WAKEUP),
        .end = PIN(PORT_SNVS, PIN_SNVS_STANDBY_REQ),
        .channel = 0,
        .value = PACK_VALUE(4, 0)
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static uint16_t calcControlIndex(uint8_t port, uint8_t number)
{
  const volatile uint32_t * const start = IMX_IOMUXC->SW_MUX_CTL_PAD_A;

  switch (port)
  {
    case PORT_AD_B0:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_AD_B0[number] - start;

    case PORT_AD_B1:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_AD_B1[number] - start;

    case PORT_B0:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_B0[number] - start;

    case PORT_B1:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_B1[number] - start;

    case PORT_EMC:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_EMC[number] - start;

    case PORT_SD_B0:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_SD_B0[number] - start;

    case PORT_SD_B1:
      return &IMX_IOMUXC->SW_MUX_CTL_PAD_GPIO_SD_B1[number] - start;

    case PORT_SNVS:
      if (number < PIN_SNVS_TEST_MODE)
      {
        return &IMX_IOMUXC_SNVS->SW_PAD_CTL_PAD[number + PIN_SNVS_TEST_MODE]
            - IMX_IOMUXC_SNVS->SW_PAD_CTL_PAD;
      }
      else
      {
        return &IMX_IOMUXC_SNVS->SW_PAD_CTL_PAD[number - PIN_SNVS_TEST_MODE]
            - IMX_IOMUXC_SNVS->SW_PAD_CTL_PAD;
      }
      break;

    default:
      break;
  }

  return UINT16_MAX;
}
/*----------------------------------------------------------------------------*/
static IMX_GPIO_Type *calcPortBase(uint8_t port, uint8_t number)
{
  static IMX_GPIO_Type * const GPIO_PORTS[] = {
      IMX_GPIO1,
      IMX_GPIO2,
      IMX_GPIO3,
      IMX_GPIO4,
      IMX_GPIO5
  };

  if (port == PORT_USB || (port == PORT_SNVS && number >= PIN_SNVS_TEST_MODE))
    return NULL;

  return GPIO_PORTS[port];
}
/*----------------------------------------------------------------------------*/
static void commonPinInit(struct Pin pin)
{
  volatile uint32_t * const gpr = getPortRemapReg(pin);

  if (gpr != NULL)
    *gpr &= ~(1UL << pin.number);

  enablePortClock(pin);
  pinSetFunction(pin, PIN_DEFAULT);
  pinSetPull(pin, PIN_NOPULL);
  pinSetType(pin, PIN_PUSHPULL);
  pinSetSlewRate(pin, PIN_SLEW_FAST);
}
/*----------------------------------------------------------------------------*/
static void enablePortClock(struct Pin pin)
{
  static const enum SysClockBranch GPIO_CLOCKS[] = {
      CLK_GPIO1, CLK_GPIO2, CLK_GPIO3, CLK_GPIO4, CLK_GPIO5
  };

  if (pin.reg != NULL && !sysClockStatus(GPIO_CLOCKS[pin.port]))
    sysClockEnable(GPIO_CLOCKS[pin.port]);
}
/*----------------------------------------------------------------------------*/
static volatile uint32_t *getMuxControlReg(struct Pin pin)
{
  if (pin.port != PORT_SNVS)
    return &IMX_IOMUXC->SW_MUX_CTL_PAD_A[pin.index];
  else
    return &IMX_IOMUXC_SNVS->SW_MUX_CTL_PAD[pin.index - PIN_SNVS_TEST_MODE];
}
/*----------------------------------------------------------------------------*/
static volatile uint32_t *getPadControlReg(struct Pin pin)
{
  if (pin.port != PORT_SNVS)
    return &IMX_IOMUXC->SW_PAD_CTL_PAD_A[pin.index];
  else
    return &IMX_IOMUXC_SNVS->SW_PAD_CTL_PAD[pin.index];
}
/*----------------------------------------------------------------------------*/
static volatile uint32_t *getPortRemapReg(struct Pin pin)
{
  static const uint8_t REMAP_INDICES[] = {26, 27, 28, 29};

  return pin.port < ARRAY_SIZE(REMAP_INDICES) ?
      &IMX_IOMUXC_GPR->GPR[REMAP_INDICES[pin.port]] : NULL;
}
/*----------------------------------------------------------------------------*/
struct Pin pinInit(PinNumber id)
{
  const struct PinDescriptor current = {
      .number = PIN_TO_OFFSET(id),
      .port = PIN_TO_PORT(id)
  };
  struct Pin pin = {
      .index = calcControlIndex(current.port, current.number)
  };

  const struct PinGroupEntry * const group = pinGroupFind(gpioPins, id, 0);

  if (group != NULL)
  {
    const struct PinDescriptor begin = {
        .number = PIN_TO_OFFSET(group->begin),
        .port = PIN_TO_PORT(group->begin)
    };

    pin.port = UNPACK_CHANNEL(group->value);
    pin.number = current.number - begin.number + UNPACK_OFFSET(group->value);
    pin.reg = (void *)calcPortBase(pin.port, pin.number);
  }
  else
  {
    pin.port = current.port;
    pin.number = current.number;
    pin.reg = NULL;
  }

  return pin;
}
/*----------------------------------------------------------------------------*/
void pinInput(struct Pin pin)
{
  if (pin.port == PORT_USB)
    return;

  commonPinInit(pin);

  volatile uint32_t * const pad = getPadControlReg(pin);
  *pad = (*pad & ~PAD_CTL_DSE_MASK) | PAD_CTL_HYS;

  if (pin.reg != NULL)
    ((IMX_GPIO_Type *)pin.reg)->GDIR &= ~(1UL << pin.number);
}
/*----------------------------------------------------------------------------*/
void pinOutput(struct Pin pin, bool value)
{
  if (pin.port == PORT_USB)
    return;

  commonPinInit(pin);

  volatile uint32_t * const pad = getPadControlReg(pin);
  *pad = (*pad & ~(PAD_CTL_DSE_MASK | PAD_CTL_HYS)) | PAD_CTL_DSE(DSE_R0);

  if (pin.reg != NULL)
  {
    pinWrite(pin, value);
    ((IMX_GPIO_Type *)pin.reg)->GDIR |= 1UL << pin.number;
  }
}
/*----------------------------------------------------------------------------*/
void pinSetDaisyChain(enum PinDaisyIndex index, uint8_t value)
{
  if (index == DAISY_UNDEFINED)
    return;

  // TODO Input Select B
  if (index < 0x100)
    IMX_IOMUXC->SELECT_INPUT_A[index] = value;
  else
    IMX_IOMUXC->SELECT_INPUT_B[index & 0xFF] = value;
}
/*----------------------------------------------------------------------------*/
void pinSetFunction(struct Pin pin, uint8_t function)
{
  if (pin.port == PORT_USB || (pin.port == PORT_SNVS
      && (pin.number >= PIN_SNVS_TEST_MODE && pin.number <= PIN_SNVS_ONOFF)))
  {
    return;
  }

  volatile uint32_t * const pad = getPadControlReg(pin);
  volatile uint32_t * const mux = getMuxControlReg(pin);
  uint32_t value = *mux & ~(MUX_CTL_MUX_MODE_MASK | MUX_CTL_SION);

  if (function == PIN_ANALOG)
  {
    /* Disable input hysteresis for analog pins */
    *pad &= ~PAD_CTL_HYS;
  }

  /* Function should not be used outside platform drivers */
  switch (function)
  {
    case PIN_DEFAULT:
    case PIN_ANALOG:
      /* Function 5 is the default for all GPIO pins */
      function = 5;
      break;

    default:
      break;
  }

  value |= MUX_CTL_MUX_MODE(function);
  *mux = value;
}
/*----------------------------------------------------------------------------*/
void pinSetPull(struct Pin pin, enum PinPull pull)
{
  if (pin.port == PORT_USB)
    return;

  volatile uint32_t * const pad = getPadControlReg(pin);
  uint32_t value = *pad & ~(PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_MASK);

  switch (pull)
  {
    case PIN_NOPULL:
      break;

    case PIN_PULLUP:
      value |= PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS(PUS_100K_PU);
      break;

    case PIN_PULLDOWN:
      value |= PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS(PUS_100K_PD);
      break;
  }

  *pad = value;
}
/*----------------------------------------------------------------------------*/
void pinSetSlewRate(struct Pin pin, enum PinSlewRate rate)
{
  if (pin.port == PORT_USB || pin.port == PORT_SNVS)
    return;

  volatile uint32_t * const pad = getPadControlReg(pin);
  uint32_t value = *pad & ~(PAD_CTL_SRE | PAD_CTL_SPEED_MASK);

  switch (rate)
  {
    case PIN_SLEW_FAST:
      value |= PAD_CTL_SRE | PAD_CTL_SPEED(SPEED_HIGH_200MHZ);
      break;

    case PIN_SLEW_NORMAL:
      value |= PAD_CTL_SRE | PAD_CTL_SPEED(SPEED_MEDIUM_100MHZ);
      break;

    case PIN_SLEW_SLOW:
      value |= PAD_CTL_SPEED(SPEED_LOW_50MHZ);
      break;
  }

  *pad = value;
}
/*----------------------------------------------------------------------------*/
void pinSetType(struct Pin pin, enum PinType type)
{
  if (pin.port == PORT_USB)
    return;

  volatile uint32_t * const pad = getPadControlReg(pin);
  uint32_t value = *pad & ~(PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_MASK);

  switch (type)
  {
    case PIN_PUSHPULL:
      value &= ~PAD_CTL_ODE;
      break;

    case PIN_OPENDRAIN:
      value |= PAD_CTL_ODE;
      break;
  }

  *pad = value;
}
