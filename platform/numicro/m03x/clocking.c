/*
 * clocking.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/m03x/clocking_defs.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
#define HIRC_FREQUENCY                48000000
#define LIRC_FREQUENCY                38400
#define RTC_FREQUENCY                 32768
#define USB_FREQUENCY                 48000000
#define TICK_RATE(frequency, latency) ((frequency) / (latency) / 1000)
/*----------------------------------------------------------------------------*/
struct ApbClockClass
{
  struct ClockClass base;
  enum ClockDivider divider;
};

struct ExtendedClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
  enum ClockBranchGroup group;
  enum ClockDivider divider;
};

struct GenericClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
  enum ClockBranchGroup group;
};
/*----------------------------------------------------------------------------*/
static uint32_t calcExtCrystalGain(uint32_t);
static bool checkClockDivider(enum ClockDivider, uint16_t);
static bool checkClockSource(enum ClockSource, enum ClockBranchGroup);
static void configCrystalPin(PinNumber);
static void configExtOscPins(bool);
static void configRtcOscPins(bool);
static uint32_t getClockDivider(enum ClockDivider);
static uint32_t getClockFrequency(enum ClockBranch, enum ClockBranchGroup);
static uint8_t getClockSource(enum ClockBranch);
static uint32_t getSourceFrequency(enum ClockBranch, enum ClockSource);
static void selectClockSource(enum ClockBranch, enum ClockSource,
    enum ClockBranchGroup);
static void setClockDivider(enum ClockDivider, uint16_t);
static void setMaxClockDivider(enum ClockDivider);
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *, const void *);
static void clockOutputDisable(const void *);

static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void intLowSpeedOscDisable(const void *);
static enum Result intLowSpeedOscEnable(const void *, const void *);
static uint32_t intLowSpeedOscFrequency(const void *);
static bool intLowSpeedOscReady(const void *);

static void rtcOscDisable(const void *);
static enum Result rtcOscEnable(const void *, const void *);
static uint32_t rtcOscFrequency(const void *);
static bool rtcOscReady(const void *);

static void sysPllDisable(const void *);
static enum Result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);

static enum Result apbBranchEnable(const void *, const void *);
static uint32_t apbBranchFrequency(const void *);

static enum Result extendedBranchEnable(const void *, const void *);
static uint32_t extendedBranchFrequency(const void *);

static enum Result genericBranchEnable(const void *, const void *);
static uint32_t genericBranchFrequency(const void *);
static bool genericBranchReady(const void *);
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ClockOutput =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = clockOutputDisable,
        .enable = clockOutputEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_CLKO,
    .group = BRANCH_GROUP_CLKO
};

const struct ClockClass * const ExternalOsc = &(const struct ClockClass){
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

const struct ClockClass * const InternalOsc = &(const struct ClockClass){
    .disable = intOscDisable,
    .enable = intOscEnable,
    .frequency = intOscFrequency,
    .ready = intOscReady
};

const struct ClockClass * const InternalLowSpeedOsc =
    &(const struct ClockClass){
    .disable = intLowSpeedOscDisable,
    .enable = intLowSpeedOscEnable,
    .frequency = intLowSpeedOscFrequency,
    .ready = intLowSpeedOscReady
};

const struct ClockClass * const RtcOsc = &(const struct ClockClass){
    .disable = rtcOscDisable,
    .enable = rtcOscEnable,
    .frequency = rtcOscFrequency,
    .ready = rtcOscReady
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const Apb0Clock =
    (const struct ClockClass *)&(const struct ApbClockClass){
    .base = {
        .disable = 0,
        .enable = apbBranchEnable,
        .frequency = apbBranchFrequency,
        .ready = genericBranchReady,
    },
    .divider = DIVIDER_APB0
};

const struct ClockClass * const Apb1Clock =
    (const struct ClockClass *)&(const struct ApbClockClass){
    .base = {
        .disable = 0,
        .enable = apbBranchEnable,
        .frequency = apbBranchFrequency,
        .ready = genericBranchReady,
    },
    .divider = DIVIDER_APB1
};

const struct ClockClass * const Bpwm0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_BPWM0,
    .group = BRANCH_GROUP_PWM
};

const struct ClockClass * const Bpwm1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_BPWM1,
    .group = BRANCH_GROUP_PWM
};

const struct ClockClass * const Pwm0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_PWM0,
    .group = BRANCH_GROUP_PWM
};

const struct ClockClass * const Pwm1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_PWM1,
    .group = BRANCH_GROUP_PWM
};

const struct ClockClass * const Qspi0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_QSPI0,
    .group = BRANCH_GROUP_ADC_SPI
};

const struct ClockClass * const Spi0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_SPI0,
    .group = BRANCH_GROUP_ADC_SPI
};

const struct ClockClass * const SysTickClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_STCLK,
    .group = BRANCH_GROUP_STCLK
};

const struct ClockClass * const Timer0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_TMR0,
    .group = BRANCH_GROUP_TMR
};

const struct ClockClass * const Timer1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_TMR1,
    .group = BRANCH_GROUP_TMR
};

const struct ClockClass * const Timer2Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_TMR2,
    .group = BRANCH_GROUP_TMR
};

const struct ClockClass * const Timer3Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_TMR3,
    .group = BRANCH_GROUP_TMR
};

const struct ClockClass * const WdtClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_WDT,
    .group = BRANCH_GROUP_WDT
};

const struct ClockClass * const WwdtClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = 0,
        .enable = genericBranchEnable,
        .frequency = genericBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_WWDT,
    .group = BRANCH_GROUP_WWDT
};

const struct ClockClass * const AdcClock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_ADC,
    .divider = DIVIDER_ADC,
    .group = BRANCH_GROUP_ADC_SPI
};

const struct ClockClass * const MainClock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_HCLK,
    .group = BRANCH_GROUP_HCLK,
    .divider = DIVIDER_HCLK
};

const struct ClockClass * const Uart0Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART0,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART0
};

const struct ClockClass * const Uart1Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART1,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART1
};

const struct ClockClass * const Uart2Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART2,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART2
};

const struct ClockClass * const Uart3Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART3,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART3
};

const struct ClockClass * const Uart4Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART4,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART4
};

const struct ClockClass * const Uart5Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART5,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART5
};

const struct ClockClass * const Uart6Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART6,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART6
};

const struct ClockClass * const Uart7Clock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_UART7,
    .group = BRANCH_GROUP_UART,
    .divider = DIVIDER_UART7
};

const struct ClockClass * const UsbClock =
    (const struct ClockClass *)&(const struct ExtendedClockClass){
    .base = {
        .disable = 0,
        .enable = extendedBranchEnable,
        .frequency = extendedBranchFrequency,
        .ready = genericBranchReady,
    },
    .branch = BRANCH_USBD,
    .group = BRANCH_GROUP_USB,
    .divider = DIVIDER_USB
};
/*----------------------------------------------------------------------------*/
static const PinNumber X32_IN_PIN = PIN(PORT_F, 5);
static const PinNumber X32_OUT_PIN = PIN(PORT_F, 4);
static const PinNumber XT1_IN_PIN = PIN(PORT_F, 3);
static const PinNumber XT1_OUT_PIN = PIN(PORT_F, 2);

static const struct PinEntry clockOutputPins[] = {
    {
        .key = PIN(PORT_A, 3),
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_B, 14),
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_C, 13),
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_D, 12),
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_F, 14),
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_F, 15),
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_G, 15),
        .channel = 0,
        .value = 14
    }, {
        .key = 0 /* End of pin function association list */
    }
};

static const struct PinEntry crystalPins[] = {
    {
        .key = PIN(PORT_F, 2),
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_F, 3),
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_F, 4),
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_F, 5),
        .channel = 0,
        .value = 10
    }, {
        .key = 0 /* End of pin function association list */
    }
};

static const enum ClockSource sourceMap[GROUP_COUNT][SOURCE_COUNT] = {
    [BRANCH_GROUP_ADC_SPI] = {
        CLOCK_EXTERNAL,
        CLOCK_PLL,
        CLOCK_APB,
        CLOCK_INTERNAL,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED
    },

    [BRANCH_GROUP_PWM] = {
        CLOCK_PLL,
        CLOCK_APB,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED
    },

    [BRANCH_GROUP_TMR] = {
        CLOCK_EXTERNAL,
        CLOCK_RTC,
        CLOCK_APB,
        CLOCK_TMR,
        CLOCK_UNDEFINED,
        CLOCK_INTERNAL_LS,
        CLOCK_UNDEFINED,
        CLOCK_INTERNAL
    },

    [BRANCH_GROUP_UART] = {
        CLOCK_EXTERNAL,
        CLOCK_PLL,
        CLOCK_RTC,
        CLOCK_INTERNAL,
        CLOCK_APB,
        CLOCK_INTERNAL_LS,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED
    },

    [BRANCH_GROUP_CLKO] = {
        CLOCK_EXTERNAL,
        CLOCK_RTC,
        CLOCK_MAIN,
        CLOCK_INTERNAL,
        CLOCK_INTERNAL_LS,
        CLOCK_INTERNAL,
        CLOCK_PLL,
        CLOCK_UNDEFINED
    },

    [BRANCH_GROUP_HCLK] = {
        CLOCK_EXTERNAL,
        CLOCK_RTC,
        CLOCK_PLL,
        CLOCK_INTERNAL_LS,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_INTERNAL
    },

    [BRANCH_GROUP_STCLK] = {
        CLOCK_EXTERNAL,
        CLOCK_RTC,
        CLOCK_EXTERNAL,   // TODO SysTick HXT_DIV2
        CLOCK_MAIN,       // TODO SysTick HCLK_DIV2
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_INTERNAL    // TODO SysTick HIRC_DIV2
    },

    [BRANCH_GROUP_USB] = {
        CLOCK_INTERNAL,
        CLOCK_PLL,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED
    },

    [BRANCH_GROUP_WDT] = {
        CLOCK_UNDEFINED,
        CLOCK_RTC,
        CLOCK_MAIN,
        CLOCK_INTERNAL_LS,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED
    },

    [BRANCH_GROUP_WWDT] = {
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_MAIN,
        CLOCK_INTERNAL_LS,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED,
        CLOCK_UNDEFINED
    }
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
uint32_t ticksPerSecond = TICK_RATE(HIRC_FREQUENCY, 3);
/*----------------------------------------------------------------------------*/
static uint32_t calcExtCrystalGain(uint32_t frequency)
{
  if (frequency <= 4000000)
    return HXTGAIN_4MHZ;
  else if (frequency <= 8000000)
    return HXTGAIN_4MHZ_8MHZ;
  else if (frequency <= 12000000)
    return HXTGAIN_8MHZ_12MHZ;
  else if (frequency <= 16000000)
    return HXTGAIN_12MHZ_16MHZ;
  else if (frequency <= 24000000)
    return HXTGAIN_16MHZ_24MHZ;
  else
    return HXTGAIN_24MHZ_32MHZ;
}
/*----------------------------------------------------------------------------*/
static bool checkClockDivider(enum ClockDivider divider, uint16_t value)
{
  return value <= MASK(EXTRACT_DIVIDER_SIZE(divider));
}
/*----------------------------------------------------------------------------*/
static bool checkClockSource(enum ClockSource source,
    enum ClockBranchGroup group)
{
  for (size_t index = 0; index < SOURCE_COUNT; ++index)
  {
    if ((enum ClockSource)sourceMap[group][index] == source)
      return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/
static void configCrystalPin(PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(crystalPins, key, 0);
  assert(pinEntry);

  const struct Pin pin = pinInit(key);
  assert(pinValid(pin));

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
static void configExtOscPins(bool bypass)
{
  configCrystalPin(XT1_IN_PIN);
  if (!bypass)
    configCrystalPin(XT1_OUT_PIN);
}
/*----------------------------------------------------------------------------*/
static void configRtcOscPins(bool bypass)
{
  configCrystalPin(X32_IN_PIN);
  if (!bypass)
    configCrystalPin(X32_OUT_PIN);
}
/*----------------------------------------------------------------------------*/
static uint32_t getClockDivider(enum ClockDivider divider)
{
  const uint32_t index = EXTRACT_DIVIDER_INDEX(divider);
  const uint32_t offset = EXTRACT_DIVIDER_OFFSET(divider);
  const uint32_t mask = BIT_FIELD(MASK(EXTRACT_DIVIDER_SIZE(divider)), offset);
  const uint32_t clkdiv = NM_CLK->CLKDIV[index];

  return (clkdiv & mask) >> offset;
}
/*----------------------------------------------------------------------------*/
static uint32_t getClockFrequency(enum ClockBranch branch,
    enum ClockBranchGroup group)
{
  const uint8_t value = getClockSource(branch);
  return getSourceFrequency(branch, sourceMap[group][value]);
}
/*----------------------------------------------------------------------------*/
static uint8_t getClockSource(enum ClockBranch branch)
{
  const uint32_t index = EXTRACT_BRANCH_INDEX(branch);
  const uint32_t offset = EXTRACT_BRANCH_OFFSET(branch);
  const uint32_t mask = BIT_FIELD(MASK(EXTRACT_BRANCH_SIZE(branch)), offset);
  const uint32_t clksel = NM_CLK->CLKSEL[index];

  return (uint8_t)((clksel & mask) >> offset);
}
/*----------------------------------------------------------------------------*/
static uint32_t getSourceFrequency(enum ClockBranch branch,
    enum ClockSource source)
{
  static const enum ClockBranch APB0_BRANCHES[] = {
      BRANCH_BPWM0,
      BRANCH_PWM0,
      BRANCH_TMR0,
      BRANCH_TMR1,
      BRANCH_QSPI0,
      BRANCH_UART0,
      BRANCH_UART2,
      BRANCH_UART4,
      BRANCH_UART6
  };

  switch (source)
  {
    case CLOCK_INTERNAL:
      if (branch == BRANCH_STCLK)
        return HIRC_FREQUENCY / 2;
      return HIRC_FREQUENCY;

    case CLOCK_INTERNAL_LS:
      return LIRC_FREQUENCY;

    case CLOCK_EXTERNAL:
      if (branch == BRANCH_STCLK)
        return extFrequency / 2;
      return extFrequency;

    case CLOCK_RTC:
      return RTC_FREQUENCY;

    case CLOCK_PLL:
      return pllFrequency;

    case CLOCK_MAIN:
    {
      const uint32_t frequency = getClockFrequency(BRANCH_HCLK,
          BRANCH_GROUP_HCLK);
      uint32_t divider = getClockDivider(DIVIDER_HCLK) + 1;

      if (branch == BRANCH_STCLK)
        divider *= 2;

      return frequency / divider;
    }

    case CLOCK_APB:
      for (size_t index = 0; index < ARRAY_SIZE(APB0_BRANCHES); ++index)
      {
        if (APB0_BRANCHES[index] == branch)
          return apbBranchFrequency(Apb0Clock);
      }

      return apbBranchFrequency(Apb1Clock);

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static void selectClockSource(enum ClockBranch branch, enum ClockSource source,
    enum ClockBranchGroup group)
{
  uint32_t value = SOURCE_RESERVED;

  for (size_t number = 0; number < SOURCE_COUNT; ++number)
  {
    if ((enum ClockSource)sourceMap[group][number] == source)
    {
      value = (uint32_t)number; // TODO Rename
      break;
    }
  }
  assert(value != SOURCE_RESERVED);

  const uint32_t index = EXTRACT_BRANCH_INDEX(branch);
  const uint32_t offset = EXTRACT_BRANCH_OFFSET(branch);
  const uint32_t mask = BIT_FIELD(MASK(EXTRACT_BRANCH_SIZE(branch)), offset);
  uint32_t clksel = NM_CLK->CLKSEL[index];

  clksel &= ~mask;
  clksel |= BIT_FIELD(value, offset);

  sysUnlockReg();
  NM_CLK->CLKSEL[index] = clksel;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static void setClockDivider(enum ClockDivider divider, uint16_t value)
{
  const uint32_t index = EXTRACT_DIVIDER_INDEX(divider);
  const uint32_t offset = EXTRACT_DIVIDER_OFFSET(divider);
  const uint32_t mask = BIT_FIELD(MASK(EXTRACT_DIVIDER_SIZE(divider)), offset);
  uint32_t clkdiv;

  clkdiv = NM_CLK->CLKDIV[index];
  clkdiv &= ~mask;
  clkdiv |= BIT_FIELD(value, offset);
  NM_CLK->CLKDIV[index] = clkdiv;
}
/*----------------------------------------------------------------------------*/
static void setMaxClockDivider(enum ClockDivider divider)
{
  const uint32_t index = EXTRACT_DIVIDER_INDEX(divider);
  const uint32_t size = EXTRACT_DIVIDER_SIZE(divider);
  uint32_t clkdiv;

  clkdiv = NM_CLK->CLKDIV[index];
  clkdiv |= BIT_FIELD(MASK(size), EXTRACT_DIVIDER_OFFSET(divider));
  NM_CLK->CLKDIV[index] = clkdiv;
}
/*----------------------------------------------------------------------------*/
static void clockOutputDisable(const void *clockBase __attribute__((unused)))
{
  NM_CLK->CLKOCTL &= ~CLKOCTL_CLKOEN;
}
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct ClockOutputConfig * const config = configBase;
  uint32_t divisor = 1;

  while ((1 << divisor) < config->divisor)
    ++divisor;
  if ((1 << divisor) != config->divisor || divisor > 15)
    return E_VALUE;

  if (!checkClockSource(config->source, BRANCH_GROUP_CLKO))
    return E_VALUE;

  const struct PinEntry * const pinEntry = pinFind(clockOutputPins,
      config->pin, 0);
  assert(pinEntry);

  const struct Pin pin = pinInit(config->pin);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  selectClockSource(BRANCH_CLKO, config->source, BRANCH_GROUP_CLKO);

  // TODO Support for CLK1HZEN
  uint32_t clkoctl = CLKOCTL_CLKOEN;

  if (divisor == 1)
    clkoctl |= CLKOCTL_DIV1EN;
  else
    clkoctl |= CLKOCTL_FREQSEL(divisor - 1);

  NM_CLK->CLKOCTL = clkoctl;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL &= ~PWRCTL_HXTEN;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  const uint32_t gain = calcExtCrystalGain(config->frequency);
  uint32_t pwrctl = NM_CLK->PWRCTL & ~PWRCTL_HXTGAIN_MASK;

  assert(config->frequency >= 4000000 && config->frequency <= 32000000);

  // TODO Detect bypass mode
  configExtOscPins(false);

  extFrequency = config->frequency;
  pwrctl |= PWRCTL_HXTGAIN(gain) | PWRCTL_HXTEN;

  sysUnlockReg();
  if (config->frequency > 12000000)
    NM_CLK->HXTFSEL &= ~HXTFSEL_HXTFSEL;
  else
    NM_CLK->HXTFSEL |= HXTFSEL_HXTFSEL;
  NM_CLK->PWRCTL = pwrctl;
  sysLockReg();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *clockBase __attribute__((unused)))
{
  return extFrequency;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *clockBase __attribute__((unused)))
{
  return extFrequency && (NM_CLK->STATUS & STATUS_HXTSTB);
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *clockBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL &= ~PWRCTL_HIRCEN;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL |= PWRCTL_HIRCEN;
  sysLockReg();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *clockBase __attribute__((unused)))
{
  return (NM_CLK->STATUS & STATUS_HIRCSTB) != 0 ? HIRC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *clockBase __attribute__((unused)))
{
  return (NM_CLK->STATUS & STATUS_HIRCSTB) != 0;
}
/*----------------------------------------------------------------------------*/
static void intLowSpeedOscDisable(const void *clockBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL &= ~PWRCTL_LIRCEN;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static enum Result intLowSpeedOscEnable(const void *clockBase
    __attribute__((unused)), const void *configBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL |= PWRCTL_LIRCEN;
  sysLockReg();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intLowSpeedOscFrequency(const void *clockBase
    __attribute__((unused)))
{
  return (NM_CLK->STATUS & STATUS_LIRCSTB) != 0 ? LIRC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intLowSpeedOscReady(const void *clockBase __attribute__((unused)))
{
  return (NM_CLK->STATUS & STATUS_LIRCSTB) != 0;
}
/*----------------------------------------------------------------------------*/
static void rtcOscDisable(const void *clockBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL &= ~PWRCTL_LXTEN;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static enum Result rtcOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct RtcOscConfig * const config = configBase;
  uint32_t pwrctl = NM_CLK->PWRCTL & ~PWRCTL_LXTGAIN_MASK;

  configRtcOscPins(config->bypass);

  if (!config->bypass)
  {
    const uint32_t gain = config->gain == RTC_GAIN_HIGH ?
        LXTGAIN_HIGH : LXTGAIN_LOW;

    pwrctl &= ~PWRCTL_LXTSELXT;
    pwrctl |= PWRCTL_LXTGAIN(gain);
  }
  else
  {
    pwrctl |= PWRCTL_LXTSELXT;
  }

  pwrctl |= PWRCTL_LXTEN;

  sysUnlockReg();
  NM_CLK->PWRCTL = pwrctl;
  sysLockReg();

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t rtcOscFrequency(const void *clockBase __attribute__((unused)))
{
  return (NM_CLK->STATUS & STATUS_LXTSTB) != 0 ? RTC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool rtcOscReady(const void *clockBase __attribute__((unused)))
{
  return (NM_CLK->STATUS & STATUS_LXTSTB) != 0;
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(const void *clockBase __attribute__((unused)))
{
  sysUnlockReg();
  NM_CLK->PWRCTL |= PLLCTL_OE | PLLCTL_PD;
  sysLockReg();
}
/*----------------------------------------------------------------------------*/
static enum Result sysPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;

  if (!config->multiplier || !config->divisor)
    return E_VALUE;
  if (config->source != CLOCK_INTERNAL && config->source != CLOCK_EXTERNAL)
    return E_VALUE;

  uint32_t fcoFrequency;
  uint32_t sourceFrequency;

  if (config->source == CLOCK_INTERNAL)
    sourceFrequency = HIRC_FREQUENCY / 4;
  else
    sourceFrequency = extFrequency;
  if (!sourceFrequency)
    return E_ERROR;

  fcoFrequency = sourceFrequency * config->multiplier;
  if (fcoFrequency < 200000000 || fcoFrequency > 500000000)
    return E_VALUE;

  uint32_t fbdiv;
  uint32_t indiv = 2;
  uint32_t outdiv;

  if (config->divisor == 1)
    outdiv = OUTDIV_DIV1;
  else if (config->divisor == 2)
    outdiv = OUTDIV_DIV2;
  else if (config->divisor == 4)
    outdiv = OUTDIV_DIV4;
  else
    return E_VALUE;

  while (sourceFrequency / indiv >= 8000000)
    ++indiv;
  if (indiv > 33 || sourceFrequency / indiv < 800000)
    return E_VALUE;

  fbdiv = fcoFrequency / (sourceFrequency / indiv);
  if (fbdiv < 2 || fbdiv > 513)
    return E_VALUE;

  uint32_t pllctl = PLLCTL_FBDIV(fbdiv - 2) | PLLCTL_INDIV(indiv - 2)
      | PLLCTL_OUTDIV(outdiv);

  if (config->source == CLOCK_INTERNAL)
    pllctl |= PLLCTL_PLLSRC;
  if (sourceFrequency > 12000000)
    pllctl |= PLLCTL_STBSEL;

  sysUnlockReg();
  NM_CLK->PLLCTL = PLLCTL_PD;
  NM_CLK->PLLCTL = PLLCTL_PD | pllctl;
  NM_CLK->PLLCTL &= ~PLLCTL_PD;
  sysLockReg();

  pllFrequency = fcoFrequency / config->divisor;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t sysPllFrequency(const void *clockBase __attribute__((unused)))
{
  return pllFrequency;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(const void *clockBase __attribute__((unused)))
{
  return pllFrequency && (NM_CLK->STATUS & STATUS_PLLSTB);
}
/*----------------------------------------------------------------------------*/
static enum Result apbBranchEnable(const void *clockBase,
    const void *configBase)
{
  const struct ApbClockConfig * const config = configBase;
  const struct ApbClockClass * const clock = clockBase;
  uint32_t divisor = 0;

  while (config->divisor > (1 << divisor))
    ++divisor;

  if (config->divisor != (1 << divisor))
    return E_VALUE;

  setClockDivider(clock->divider, divisor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t apbBranchFrequency(const void *clockBase)
{
  const struct ExtendedClockClass * const clock = clockBase;
  const uint32_t frequency = getClockFrequency(BRANCH_HCLK, BRANCH_GROUP_HCLK);
  const uint32_t ahbDivider = getClockDivider(DIVIDER_HCLK) + 1;
  const uint32_t apbDivider = ahbDivider << getClockDivider(clock->divider);

  return frequency / apbDivider;
}
/*----------------------------------------------------------------------------*/
static enum Result extendedBranchEnable(const void *clockBase,
    const void *configBase)
{
  const struct ExtendedClockConfig * const config = configBase;
  const struct ExtendedClockClass * const clock = clockBase;
  uint16_t divisor = config->divisor;

  if (!divisor)
    return E_VALUE;
  --divisor;

  if (!checkClockDivider(clock->divider, divisor))
    return E_VALUE;
  if (!checkClockSource(config->source, clock->group))
    return E_VALUE;

  if (clock->branch == BRANCH_HCLK)
    sysFlashLatencyReset();

  setMaxClockDivider(clock->divider);
  selectClockSource(clock->branch, config->source, clock->group);
  setClockDivider(clock->divider, divisor);

  if (clock->branch == BRANCH_HCLK)
  {
    uint32_t frequency = getClockFrequency(BRANCH_HCLK, BRANCH_GROUP_HCLK);

    frequency = frequency / (divisor + 1);
    sysFlashLatencyUpdate(frequency);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extendedBranchFrequency(const void *clockBase)
{
  const struct ExtendedClockClass * const clock = clockBase;
  const uint16_t divisor = getClockDivider(clock->divider) + 1;

  return getClockFrequency(clock->branch, clock->group) / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result genericBranchEnable(const void *clockBase,
    const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  const struct GenericClockClass * const clock = clockBase;

  if (checkClockSource(config->source, clock->group))
  {
    selectClockSource(clock->branch, config->source, clock->group);
    return E_OK;
  }
  else
    return E_VALUE;
}
/*----------------------------------------------------------------------------*/
static uint32_t genericBranchFrequency(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  return getClockFrequency(clock->branch, clock->group);
}
/*----------------------------------------------------------------------------*/
static bool genericBranchReady(const void *clockBase __attribute__((unused)))
{
  return true;
}
