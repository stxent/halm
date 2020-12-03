/*
 * clocking.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/delay.h>
#include <halm/platform/lpc/lpc43xx/clocking.h>
#include <halm/platform/lpc/lpc43xx/clocking_defs.h>
#include <halm/platform/lpc/lpc43xx/system.h>
#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
struct GenericClockClass
{
  struct ClockClass base;
  enum ClockBranch branch;
};

struct GenericDividerClass
{
  struct ClockClass base;
  enum ClockSource channel;
};

struct GenericPllClass
{
  struct ClockClass base;
  enum ClockSource channel;
};
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY     12000000
#define RTC_OSC_FREQUENCY     32768
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcBranchReg(enum ClockBranch);
static inline volatile uint32_t *calcDividerReg(enum ClockSource);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static uint32_t getSourceFrequency(enum ClockSource);
static unsigned int pll0ComputeMdec(unsigned int);
static unsigned int pll0ComputeNdec(unsigned int);
static unsigned int pll0ComputePdec(unsigned int);
static unsigned int pll0ComputeSelI(unsigned int);
static unsigned int pll0ComputeSelP(unsigned int);
/*----------------------------------------------------------------------------*/
static void genericDividerDisable(const void *);
static enum Result genericDividerEnable(const void *, const void *);
static uint32_t genericDividerFrequency(const void *);
static bool genericDividerReady(const void *);

static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void rtcOscDisable(const void *);
static enum Result rtcOscEnable(const void *, const void *);
static uint32_t rtcOscFrequency(const void *);
static bool rtcOscReady(const void *);

static void pll0ClockDisable(const void *);
static enum Result pll0ClockEnable(const void *, const void *);
static uint32_t pll0ClockFrequency(const void *);
static bool pll0ClockReady(const void *);

static void pll1ClockDisable(const void *);
static enum Result pll1ClockEnable(const void *, const void *);
static uint32_t pll1ClockFrequency(const void *);
static bool pll1ClockReady(const void *);

static enum Result clockOutputEnable(const void *, const void *);

static void genericClockDisable(const void *);
static enum Result genericClockEnable(const void *, const void *);
static uint32_t genericClockFrequency(const void *);
static bool genericClockReady(const void *);
/*----------------------------------------------------------------------------*/
const struct ClockClass * const DividerA =
    (const struct ClockClass *)&(const struct GenericDividerClass){
    .base = {
        .disable = genericDividerDisable,
        .enable = genericDividerEnable,
        .frequency = genericDividerFrequency,
        .ready = genericDividerReady
    },
    .channel = CLOCK_IDIVA
};

const struct ClockClass * const DividerB =
    (const struct ClockClass *)&(const struct GenericDividerClass){
    .base = {
        .disable = genericDividerDisable,
        .enable = genericDividerEnable,
        .frequency = genericDividerFrequency,
        .ready = genericDividerReady
    },
    .channel = CLOCK_IDIVB
};

const struct ClockClass * const DividerC =
    (const struct ClockClass *)&(const struct GenericDividerClass){
    .base = {
        .disable = genericDividerDisable,
        .enable = genericDividerEnable,
        .frequency = genericDividerFrequency,
        .ready = genericDividerReady
    },
    .channel = CLOCK_IDIVC
};

const struct ClockClass * const DividerD =
    (const struct ClockClass *)&(const struct GenericDividerClass){
    .base = {
        .disable = genericDividerDisable,
        .enable = genericDividerEnable,
        .frequency = genericDividerFrequency,
        .ready = genericDividerReady
    },
    .channel = CLOCK_IDIVD
};

const struct ClockClass * const DividerE =
    (const struct ClockClass *)&(const struct GenericDividerClass){
    .base = {
        .disable = genericDividerDisable,
        .enable = genericDividerEnable,
        .frequency = genericDividerFrequency,
        .ready = genericDividerReady
    },
    .channel = CLOCK_IDIVE
};
/*----------------------------------------------------------------------------*/
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

const struct ClockClass * const RtcOsc = &(const struct ClockClass){
    .disable = rtcOscDisable,
    .enable = rtcOscEnable,
    .frequency = rtcOscFrequency,
    .ready = rtcOscReady
};

const struct ClockClass * const AudioPll =
    (const struct ClockClass *)&(const struct GenericPllClass){
    .base = {
        .disable = pll0ClockDisable,
        .enable = pll0ClockEnable,
        .frequency = pll0ClockFrequency,
        .ready = pll0ClockReady
    },
    .channel = CLOCK_AUDIO_PLL
};

const struct ClockClass * const UsbPll =
    (const struct ClockClass *)&(const struct GenericPllClass){
    .base = {
        .disable = pll0ClockDisable,
        .enable = pll0ClockEnable,
        .frequency = pll0ClockFrequency,
        .ready = pll0ClockReady
    },
    .channel = CLOCK_USB_PLL
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = pll1ClockDisable,
    .enable = pll1ClockEnable,
    .frequency = pll1ClockFrequency,
    .ready = pll1ClockReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const MainClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_M4
};

const struct ClockClass * const Usb0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_USB0
};

const struct ClockClass * const Usb1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_USB1
};

const struct ClockClass * const PeriphClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_PERIPH
};

const struct ClockClass * const Apb1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_APB1
};

const struct ClockClass * const Apb3Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_APB3
};

const struct ClockClass * const SpifiClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_SPIFI
};

const struct ClockClass * const SpiClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_SPI
};

const struct ClockClass * const PhyRxClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_PHY_RX
};

const struct ClockClass * const PhyTxClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_PHY_TX
};

const struct ClockClass * const LcdClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_LCD
};

const struct ClockClass * const AdcHsClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_ADCHS
};

const struct ClockClass * const SdioClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_SDIO
};

const struct ClockClass * const Ssp0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_SSP0
};

const struct ClockClass * const Ssp1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_SSP1
};

const struct ClockClass * const Usart0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_USART0
};

const struct ClockClass * const Uart1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_UART1
};

const struct ClockClass * const Usart2Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_USART2
};

const struct ClockClass * const Usart3Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_USART3
};

const struct ClockClass * const AudioClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = genericClockEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_AUDIO
};

const struct ClockClass * const OutClock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = clockOutputEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_OUT
};

const struct ClockClass * const CguOut0Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = clockOutputEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT0
};

const struct ClockClass * const CguOut1Clock =
    (const struct ClockClass *)&(const struct GenericClockClass){
    .base = {
        .disable = genericClockDisable,
        .enable = clockOutputEnable,
        .frequency = genericClockFrequency,
        .ready = genericClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT1
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry clockOutputPins[] = {
    {
        .key = PIN(PORT_1, 19), /* CLKOUT */
        .channel = CLOCK_BASE_OUT,
        .value = 4
    }, {
        .key = PIN(PORT_3, 3), /* CGU_OUT1 */
        .channel = CLOCK_BASE_CGU_OUT1,
        .value = 4
    }, {
        .key = PIN(PORT_8, 8), /* CGU_OUT0 */
        .channel = CLOCK_BASE_CGU_OUT0,
        .value = 6
    }, {
        .key = PIN(PORT_A, 0), /* CGU_OUT1 */
        .channel = CLOCK_BASE_CGU_OUT1,
        .value = 6
    }, {
        .key = PIN(PORT_CLK, 0), /* CLKOUT */
        .channel = CLOCK_BASE_OUT,
        .value = 1
    }, {
        .key = PIN(PORT_CLK, 1), /* CLKOUT */
        .channel = CLOCK_BASE_OUT,
        .value = 1
    }, {
        .key = PIN(PORT_CLK, 1), /* CGU_OUT0 */
        .channel = CLOCK_BASE_CGU_OUT0,
        .value = 5
    }, {
        .key = PIN(PORT_CLK, 2), /* CLKOUT */
        .channel = CLOCK_BASE_OUT,
        .value = 1
    }, {
        .key = PIN(PORT_CLK, 3), /* CLKOUT */
        .channel = CLOCK_BASE_OUT,
        .value = 1
    }, {
        .key = PIN(PORT_CLK, 3), /* CGU_OUT1 */
        .channel = CLOCK_BASE_CGU_OUT1,
        .value = 5
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pll0AudioFrequency = 0;
static uint32_t pll0UsbFrequency = 0;
static uint32_t pll1Frequency = 0;
uint32_t ticksPerSecond = TICK_RATE(INT_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcBranchReg(enum ClockBranch branch)
{
  return &LPC_CGU->BASE_USB0_CLK + branch;
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcDividerReg(enum ClockSource source)
{
  return &LPC_CGU->IDIVA_CTRL + (source - CLOCK_IDIVA);
}
/*----------------------------------------------------------------------------*/
static inline LPC_CGU_PLL0_Type *calcPllReg(enum ClockSource source)
{
  return source == CLOCK_AUDIO_PLL ? &LPC_CGU->PLL0AUDIO : &LPC_CGU->PLL0USB;
}
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Select safe setting */
  sysFlashLatencyUpdate(10);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  static const uint8_t frequencyToClocks[] = {
      21, 43, 64, 86, 107, 129, 150, 172, 193
  };

  const unsigned int encodedFrequency = frequency / 1000000;
  size_t index;

  for (index = 0; index < ARRAY_SIZE(frequencyToClocks); ++index)
  {
    if (encodedFrequency <= frequencyToClocks[index])
      break;
  }

  sysFlashLatencyUpdate(index + 1);
}
/*----------------------------------------------------------------------------*/
static uint32_t getSourceFrequency(enum ClockSource source)
{
  switch (source)
  {
    case CLOCK_RTC:
      return rtcOscFrequency(RtcOsc);

    case CLOCK_INTERNAL:
      return INT_OSC_FREQUENCY;

    case CLOCK_EXTERNAL:
      return extFrequency;

    case CLOCK_USB_PLL:
      return pll0UsbFrequency;

    case CLOCK_AUDIO_PLL:
      return pll0AudioFrequency;

    case CLOCK_PLL:
      return pll1Frequency;

    case CLOCK_IDIVA:
      return genericDividerFrequency(DividerA);

    case CLOCK_IDIVB:
      return genericDividerFrequency(DividerB);

    case CLOCK_IDIVC:
      return genericDividerFrequency(DividerC);

    case CLOCK_IDIVD:
      return genericDividerFrequency(DividerD);

    case CLOCK_IDIVE:
      return genericDividerFrequency(DividerE);

    default:
      /* Unknown clock source */
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static unsigned int pll0ComputeMdec(unsigned int msel)
{
  /*
   * Compute multiplier MDEC from MSEL.
   * The valid range of MSEL is from 1 to 2^15.
   */

  switch (msel)
  {
    case 0:
      return 0xFFFFFFFFUL;

    case 1:
      return 0x18003;

    case 2:
      return 0x10003;

    default:
    {
      unsigned int x = 0x4000;

      for (unsigned int im = msel; im <= (1 << 15); ++im)
        x = (((x ^ x >> 1) & 1) << 14) | (x >> 1 & 0x3FFF);

      return x;
    }
  }
}
/*----------------------------------------------------------------------------*/
static unsigned int pll0ComputeNdec(unsigned int nsel)
{
  /*
   * Compute pre-divider NDEC from NSEL.
   * The valid range of NSEL is from 1 to 2^8.
   */

  switch (nsel)
  {
    case 0:
      return 0xFFFFFFFFUL;

    case 1:
      return 0x302;

    case 2:
      return 0x202;

    default:
    {
      unsigned int x = 0x80;

      for (unsigned int in = nsel; in <= (1 << 8); ++in)
        x = (((x ^ x >> 2 ^ x >> 3 ^ x >> 4) & 1) << 7) | (x >> 1 & 0x7F);

      return x;
    }
  }
}
/*----------------------------------------------------------------------------*/
static unsigned int pll0ComputePdec(unsigned int psel)
{
  /*
   * Compute post-divider PDEC from PSEL.
   * The valid range of PSEL is from 1 to 2^5
   */

  switch (psel)
  {
    case 0:
      return 0xFFFFFFFFUL;

    case 1:
      return 0x62;

    case 2:
      return 0x42;

    default:
    {
      unsigned int x = 0x10;

      for (unsigned int ip = psel; ip <= (1 << 5); ++ip)
        x = (((x ^ x >> 2) & 1) << 4) | (x >> 1 & 0xF);

      return x;
    }
  }
}
/*----------------------------------------------------------------------------*/
static unsigned int pll0ComputeSelI(unsigned int msel)
{
  /* Bandwidth: compute SELI from MSEL */

  if (msel > 16384)
    return 1;
  else if (msel > 8192)
    return 2;
  else if (msel > 2048)
    return 4;
  else if (msel >= 501)
    return 8;
  else if (msel >= 60)
    return 4 * (1024 / (msel + 9));
  else
    return (msel & 0x3C) + 4;
}
/*----------------------------------------------------------------------------*/
static unsigned int pll0ComputeSelP(unsigned int msel)
{
  /* Bandwidth: compute SELP from MSEL */
  return msel < 60 ? ((msel >> 1) + 1) : 31;
}
/*----------------------------------------------------------------------------*/
static void genericDividerDisable(const void *clockBase)
{
  const struct GenericDividerClass * const clock = clockBase;
  *calcDividerReg(clock->channel) |= IDIV_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result genericDividerEnable(const void *clockBase,
    const void *configBase)
{
  const struct GenericDividerClass * const clock = clockBase;
  const struct GenericDividerConfig * const config = configBase;
  volatile uint32_t * const reg = calcDividerReg(clock->channel);

  assert(config->divisor);
  assert(clock->channel != CLOCK_IDIVA || config->divisor <= 4);
  assert(clock->channel != CLOCK_IDIVE || config->divisor <= 256);
  assert(clock->channel == CLOCK_IDIVA || clock->channel == CLOCK_IDIVE
      || config->divisor <= 16);

  *reg |= IDIV_AUTOBLOCK;
  *reg = (*reg & ~(IDIV_DIVIDER_MASK | IDIV_CLK_SEL_MASK))
      | IDIV_DIVIDER(config->divisor - 1) | IDIV_CLK_SEL(config->source);
  *reg &= ~IDIV_PD;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t genericDividerFrequency(const void *clockBase)
{
  const struct GenericDividerClass * const clock = clockBase;
  const uint32_t value = *calcDividerReg(clock->channel);
  const uint32_t divider = IDIV_DIVIDER_VALUE(value) + 1;
  const uint32_t frequency = getSourceFrequency(IDIV_CLK_SEL_VALUE(value));
  return frequency / divider;
}
/*----------------------------------------------------------------------------*/
static bool genericDividerReady(const void *clockBase)
{
  const struct GenericDividerClass * const clock = clockBase;
  return !(*calcDividerReg(clock->channel) & IDIV_PD);
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  LPC_CGU->XTAL_OSC_CTRL &= ~XTAL_ENABLE;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  uint32_t buffer = XTAL_ENABLE;

  assert(config->frequency >= 1000000 && config->frequency <= 25000000);

  if (config->bypass)
    buffer |= XTAL_BYPASS;

  /* Bypass bit should not be changed in one write-action with the enable bit */
  LPC_CGU->XTAL_OSC_CTRL = buffer;

  if (config->frequency > 15000000)
    buffer |= XTAL_HF;

  buffer &= ~XTAL_ENABLE;
  LPC_CGU->XTAL_OSC_CTRL = buffer;

  extFrequency = config->frequency;

  /*
   * There is no status register so wait for 250 microseconds
   * as recommended in the user manual.
   */
  udelay(250);

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
  return extFrequency && !(LPC_CGU->XTAL_OSC_CTRL & XTAL_ENABLE);
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *clockBase __attribute__((unused)))
{
  /* Unsupported */
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *clockBase __attribute__((unused)))
{
  return INT_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *clockBase __attribute__((unused)))
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void rtcOscDisable(const void *clockBase __attribute__((unused)))
{
  /* Disable and reset clock, disable outputs */
  LPC_CREG->CREG0 = (LPC_CREG->CREG0 & ~(CREG0_EN1KHZ | CREG0_EN32KHZ))
      | CREG0_PD32KHZ | CREG0_RESET32KHZ;
}
/*----------------------------------------------------------------------------*/
static enum Result rtcOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  /* Enable and reset clock */
  LPC_CREG->CREG0 = (LPC_CREG->CREG0 & ~CREG0_PD32KHZ) | CREG0_RESET32KHZ;
  /* Enable clock outputs and disable reset */
  LPC_CREG->CREG0 = (LPC_CREG->CREG0 & ~CREG0_RESET32KHZ)
      | CREG0_EN1KHZ | CREG0_EN32KHZ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t rtcOscFrequency(const void *clockBase __attribute__((unused)))
{
  return rtcOscReady(clockBase) ? RTC_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool rtcOscReady(const void *clockBase __attribute__((unused)))
{
  /* There is no convenient way to find out that the oscillator is running */
  return !(LPC_CREG->CREG0 & (CREG0_PD32KHZ | CREG0_RESET32KHZ));
}
/*----------------------------------------------------------------------------*/
static void pll0ClockDisable(const void *clockBase)
{
  const struct GenericPllClass * const clock = clockBase;
  calcPllReg(clock->channel)->CTRL |= BASE_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result pll0ClockEnable(const void *clockBase,
    const void *configBase)
{
  const struct GenericPllClass * const clock = clockBase;
  const struct PllConfig * const config = configBase;
  assert(config->source != CLOCK_AUDIO_PLL && config->source != CLOCK_USB_PLL);

  if (!config->divisor || config->divisor > 64)
    return E_VALUE;
  if (config->divisor > 1 && (config->divisor & 1) != 0)
    return E_VALUE;
  if (!config->multiplier || config->multiplier > 32768)
    return E_VALUE;

  const bool multiplierIsEven = (config->multiplier & 1) == 0;
  const unsigned int psel = config->divisor > 1 ? config->divisor >> 1 : 0;
  const unsigned int nsel = multiplierIsEven ? 1 : 2;
  const unsigned int msel = multiplierIsEven ?
      config->multiplier >> 1 : config->multiplier;

  const bool directInput = nsel == 1;
  const bool directOutput = psel == 0;

  const uint32_t inputFrequency = getSourceFrequency(config->source);
  if (!inputFrequency)
    return E_IDLE;

  const uint32_t ccoFrequency = (inputFrequency / nsel) * (msel << 1);
  const uint32_t expectedFrequency =
      psel ? ccoFrequency / (psel << 1) : ccoFrequency;

  /* Check CCO frequency */
  if (ccoFrequency < 275000000 || ccoFrequency > 550000000)
    return E_VALUE;

  const uint32_t mDivValue = PLL0_MDIV_MDEC(pll0ComputeMdec(msel))
      | PLL0_MDIV_SELP(pll0ComputeSelP(msel))
      | PLL0_MDIV_SELI(pll0ComputeSelI(msel));
  const uint32_t npDivValue = PLL0_NP_DIV_PDEC(pll0ComputePdec(psel))
      | PLL0_NP_DIV_NDEC(pll0ComputeNdec(nsel));
  uint32_t controlValue = PLL0_CTRL_CLKEN | BASE_AUTOBLOCK
      | BASE_CLK_SEL(config->source);

  if (directInput)
    controlValue |= PLL0_CTRL_DIRECTI;
  if (directOutput)
    controlValue |= PLL0_CTRL_DIRECTO;
  if (clock->channel == CLOCK_AUDIO_PLL)
    controlValue |= PLL0_SEL_EXT | PLL0_MOD_PD;

  LPC_CGU_PLL0_Type * const reg = calcPllReg(clock->channel);

  reg->CTRL |= BASE_PD | BASE_AUTOBLOCK;
  reg->MDIV = mDivValue;
  reg->NP_DIV = npDivValue;
  reg->CTRL = controlValue;

  if (clock->channel == CLOCK_AUDIO_PLL)
    pll0AudioFrequency = expectedFrequency;
  else
    pll0UsbFrequency = expectedFrequency;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll0ClockFrequency(const void *clockBase)
{
  const struct GenericPllClass * const clock = clockBase;

  return clock->channel == CLOCK_AUDIO_PLL ?
      pll0AudioFrequency : pll0UsbFrequency;
}
/*----------------------------------------------------------------------------*/
static bool pll0ClockReady(const void *clockBase)
{
  const struct GenericPllClass * const clock = clockBase;
  const uint32_t frequency = pll0ClockFrequency(clock);

  return frequency && calcPllReg(clock->channel)->STAT & PLL0_STAT_LOCK;
}
/*----------------------------------------------------------------------------*/
static void pll1ClockDisable(const void *clockBase __attribute__((unused)))
{
  LPC_CGU->PLL1_CTRL |= BASE_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result pll1ClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config->source != CLOCK_PLL);

  if (!config->divisor)
    return E_VALUE;
  if (!config->multiplier || config->multiplier > 256)
    return E_VALUE;

  const unsigned int psel = countLeadingZeros32(reverseBits32(config->divisor));
  const unsigned int nsel = config->divisor >> psel;

  if (psel > 4 || nsel > 4 || nsel << psel != config->divisor)
    return E_VALUE;

  const uint32_t inputFrequency = getSourceFrequency(config->source);
  if (!inputFrequency)
    return E_ERROR;

  const uint32_t ccoFrequency = (inputFrequency / nsel) * config->multiplier;
  const uint32_t expectedFrequency = ccoFrequency >> psel;

  /* Check CCO frequency */
  if (ccoFrequency < 156000000 || ccoFrequency > 320000000)
    return E_VALUE;

  uint32_t controlValue = BASE_AUTOBLOCK | BASE_CLK_SEL(config->source)
      | PLL1_CTRL_NSEL(nsel - 1) | PLL1_CTRL_MSEL(config->multiplier - 1);
  const bool direct = psel == 0;

  if (!direct)
    controlValue |= PLL1_CTRL_PSEL(psel - 1);

  if (expectedFrequency > 110000000)
  {
    /* No division allowed */
    if (!direct)
      return E_VALUE;

    /* Start at mid-range frequency by dividing output clock */
    LPC_CGU->PLL1_CTRL |= BASE_PD | BASE_AUTOBLOCK;
    LPC_CGU->PLL1_CTRL = controlValue;

    /* Wait for PLL to lock */
    unsigned int timeout = 100000;

    while (!(LPC_CGU->PLL1_STAT & PLL1_STAT_LOCK) && --timeout)
      udelay(10);

    if (!timeout)
      return E_ERROR;

    /* User manual recommends to wait for 50 microseconds */
    udelay(50);

    /* Double the output frequency by enabling direct output */
    LPC_CGU->PLL1_CTRL |= PLL1_CTRL_DIRECT;
  }
  else
  {
    if (direct)
      controlValue |= PLL1_CTRL_FBSEL | PLL1_CTRL_DIRECT;

    LPC_CGU->PLL1_CTRL |= BASE_PD | BASE_AUTOBLOCK;
    LPC_CGU->PLL1_CTRL = controlValue;
  }

  pll1Frequency = expectedFrequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll1ClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return pll1Frequency;
}
/*----------------------------------------------------------------------------*/
static bool pll1ClockReady(const void *clockBase __attribute__((unused)))
{
  return pll1Frequency && (LPC_CGU->PLL1_STAT & PLL1_STAT_LOCK);
}
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *clockBase,
    const void *configBase)
{
  const struct GenericClockClass * const clock = clockBase;
  const struct ClockOutputConfig * const config = configBase;
  const struct GenericClockConfig baseConfig = {
      .source = config->source
  };

  const struct PinEntry * const pinEntry = pinFind(clockOutputPins,
      config->pin, clock->branch);
  assert(pinEntry);

  const struct Pin pin = pinInit(config->pin);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  return genericClockEnable(clock, &baseConfig);
}
/*----------------------------------------------------------------------------*/
static void genericClockDisable(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  *calcBranchReg(clock->branch) |= BASE_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result genericClockEnable(const void *clockBase,
    const void *configBase)
{
  const struct GenericClockClass * const clock = clockBase;
  const struct GenericClockConfig * const config = configBase;
  volatile uint32_t * const reg = calcBranchReg(clock->branch);

  if (clock->branch == CLOCK_BASE_M4)
    flashLatencyReset();

  *reg |= BASE_AUTOBLOCK;
  *reg = (*reg & ~BASE_CLK_SEL_MASK) | BASE_CLK_SEL(config->source);
  *reg &= ~BASE_PD;

  if (clock->branch == CLOCK_BASE_M4)
  {
    const uint32_t frequency = genericClockFrequency(clockBase);

    flashLatencyUpdate(frequency);
    ticksPerSecond = TICK_RATE(frequency);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t genericClockFrequency(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  return getSourceFrequency(BASE_CLK_SEL_VALUE(*calcBranchReg(clock->branch)));
}
/*----------------------------------------------------------------------------*/
static bool genericClockReady(const void *clockBase)
{
  const struct GenericClockClass * const clock = clockBase;
  return !(*calcBranchReg(clock->branch) & BASE_PD);
}
