/*
 * clocking.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stddef.h>
#include <halm/delay.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/clocking_defs.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <halm/platform/nxp/lpc43xx/system_defs.h>
#include <halm/platform/platform_defs.h>
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
static void commonDividerDisable(const void *);
static enum Result commonDividerEnable(const void *, const void *);
static uint32_t commonDividerFrequency(const void *);
static bool commonDividerReady(const void *);

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

static void pll0UsbClockDisable(const void *);
static enum Result pll0UsbClockEnable(const void *, const void *);
static uint32_t pll0UsbClockFrequency(const void *);
static bool pll0UsbClockReady(const void *);

static void pll1ClockDisable(const void *);
static enum Result pll1ClockEnable(const void *, const void *);
static uint32_t pll1ClockFrequency(const void *);
static bool pll1ClockReady(const void *);

static enum Result clockOutputEnable(const void *, const void *);

static void commonClockDisable(const void *);
static enum Result commonClockEnable(const void *, const void *);
static uint32_t commonClockFrequency(const void *);
static bool commonClockReady(const void *);
/*----------------------------------------------------------------------------*/
static const struct CommonDividerClass dividerATable = {
    .base = {
        .disable = commonDividerDisable,
        .enable = commonDividerEnable,
        .frequency = commonDividerFrequency,
        .ready = commonDividerReady
    },
    .channel = CLOCK_IDIVA
};

static const struct CommonDividerClass dividerBTable = {
    .base = {
        .disable = commonDividerDisable,
        .enable = commonDividerEnable,
        .frequency = commonDividerFrequency,
        .ready = commonDividerReady
    },
    .channel = CLOCK_IDIVB
};

static const struct CommonDividerClass dividerCTable = {
    .base = {
        .disable = commonDividerDisable,
        .enable = commonDividerEnable,
        .frequency = commonDividerFrequency,
        .ready = commonDividerReady
    },
    .channel = CLOCK_IDIVC
};

static const struct CommonDividerClass dividerDTable = {
    .base = {
        .disable = commonDividerDisable,
        .enable = commonDividerEnable,
        .frequency = commonDividerFrequency,
        .ready = commonDividerReady
    },
    .channel = CLOCK_IDIVD
};

static const struct CommonDividerClass dividerETable = {
    .base = {
        .disable = commonDividerDisable,
        .enable = commonDividerEnable,
        .frequency = commonDividerFrequency,
        .ready = commonDividerReady
    },
    .channel = CLOCK_IDIVE
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

static const struct ClockClass intOscTable = {
    .disable = intOscDisable,
    .enable = intOscEnable,
    .frequency = intOscFrequency,
    .ready = intOscReady
};

static const struct ClockClass rtcOscTable = {
    .disable = rtcOscDisable,
    .enable = rtcOscEnable,
    .frequency = rtcOscFrequency,
    .ready = rtcOscReady
};

static const struct ClockClass pll0UsbTable = {
    .disable = pll0UsbClockDisable,
    .enable = pll0UsbClockEnable,
    .frequency = pll0UsbClockFrequency,
    .ready = pll0UsbClockReady
};

static const struct ClockClass pll1Table = {
    .disable = pll1ClockDisable,
    .enable = pll1ClockEnable,
    .frequency = pll1ClockFrequency,
    .ready = pll1ClockReady
};
/*----------------------------------------------------------------------------*/
static const struct CommonClockClass mainClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_M4
};

static const struct CommonClockClass usb0ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USB0
};

static const struct CommonClockClass usb1ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USB1
};

static const struct CommonClockClass periphClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_PERIPH
};

static const struct CommonClockClass apb1ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_APB1
};

static const struct CommonClockClass apb3ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_APB3
};

static const struct CommonClockClass spifiClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SPIFI
};

static const struct CommonClockClass spiClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SPI
};

static const struct CommonClockClass phyRxClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_PHY_RX
};

static const struct CommonClockClass phyTxClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_PHY_TX
};

static const struct CommonClockClass lcdClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_LCD
};

static const struct CommonClockClass adcHsClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_ADCHS
};

static const struct CommonClockClass sdioClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SDIO
};

static const struct CommonClockClass ssp0ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SSP0
};

static const struct CommonClockClass ssp1ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SSP1
};

static const struct CommonClockClass usart0ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USART0
};

static const struct CommonClockClass uart1ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_UART1
};

static const struct CommonClockClass usart2ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USART2
};

static const struct CommonClockClass usart3ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USART3
};

static const struct CommonClockClass audioClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_AUDIO
};

static const struct CommonClockClass outClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = clockOutputEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_OUT
};

static const struct CommonClockClass cguOut0ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = clockOutputEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT0
};

static const struct CommonClockClass cguOut1ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = clockOutputEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
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
const struct CommonDividerClass * const DividerA = &dividerATable;
const struct CommonDividerClass * const DividerB = &dividerBTable;
const struct CommonDividerClass * const DividerC = &dividerCTable;
const struct CommonDividerClass * const DividerD = &dividerDTable;
const struct CommonDividerClass * const DividerE = &dividerETable;
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const RtcOsc = &rtcOscTable;
const struct ClockClass * const UsbPll = &pll0UsbTable;
const struct ClockClass * const SystemPll = &pll1Table;
/*----------------------------------------------------------------------------*/
const struct CommonClockClass * const MainClock = &mainClockTable;
const struct CommonClockClass * const Usb0Clock = &usb0ClockTable;
const struct CommonClockClass * const Usb1Clock = &usb1ClockTable;
const struct CommonClockClass * const PeriphClock = &periphClockTable;
const struct CommonClockClass * const Apb1Clock = &apb1ClockTable;
const struct CommonClockClass * const Apb3Clock = &apb3ClockTable;
const struct CommonClockClass * const SpifiClock = &spifiClockTable;
const struct CommonClockClass * const SpiClock = &spiClockTable;
const struct CommonClockClass * const PhyRxClock = &phyRxClockTable;
const struct CommonClockClass * const PhyTxClock = &phyTxClockTable;
const struct CommonClockClass * const LcdClock = &lcdClockTable;
const struct CommonClockClass * const AdcHsClock = &adcHsClockTable;
const struct CommonClockClass * const SdioClock = &sdioClockTable;
const struct CommonClockClass * const Ssp0Clock = &ssp0ClockTable;
const struct CommonClockClass * const Ssp1Clock = &ssp1ClockTable;
const struct CommonClockClass * const Usart0Clock = &usart0ClockTable;
const struct CommonClockClass * const Uart1Clock = &uart1ClockTable;
const struct CommonClockClass * const Usart2Clock = &usart2ClockTable;
const struct CommonClockClass * const Usart3Clock = &usart3ClockTable;
const struct CommonClockClass * const AudioClock = &audioClockTable;
const struct CommonClockClass * const OutClock = &outClockTable;
const struct CommonClockClass * const CguOut0Clock = &cguOut0ClockTable;
const struct CommonClockClass * const CguOut1Clock = &cguOut1ClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pll0UsbFrequency = 0;
static uint32_t pll0AudioFrequency = 0;
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
      return commonDividerFrequency(DividerA);

    case CLOCK_IDIVB:
      return commonDividerFrequency(DividerB);

    case CLOCK_IDIVC:
      return commonDividerFrequency(DividerC);

    case CLOCK_IDIVD:
      return commonDividerFrequency(DividerD);

    case CLOCK_IDIVE:
      return commonDividerFrequency(DividerE);

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
      return 0xFFFFFFFF;

    case 1:
      return 0x18003;

    case 2:
      return 0x10003;

    default:
    {
      unsigned x = 0x4000;

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
      return 0xFFFFFFFF;

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
      return 0xFFFFFFFF;

    case 1:
      return 0x62;

    case 2:
      return 0x42;

    default:
    {
      unsigned int x = 0x10;

      //TODO Constants PLL0_MSEL_MAX etc
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

  if (msel < 60)
    return (msel >> 1) + 1;
  else
    return 31;
}
/*----------------------------------------------------------------------------*/
static void commonDividerDisable(const void *clockBase)
{
  const struct CommonDividerClass * const clock = clockBase;
  volatile uint32_t * const reg = calcDividerReg(clock->channel);

  *reg |= IDIV_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result commonDividerEnable(const void *clockBase,
    const void *configBase)
{
  const struct CommonDividerClass * const clock = clockBase;
  const struct CommonDividerConfig * const config = configBase;
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
static uint32_t commonDividerFrequency(const void *clockBase)
{
  const struct CommonDividerClass * const clock = clockBase;
  const volatile uint32_t * const reg = calcDividerReg(clock->channel);
  const uint32_t divider = IDIV_DIVIDER_VALUE(*reg) + 1;
  const uint32_t frequency = getSourceFrequency(IDIV_CLK_SEL_VALUE(*reg));

  return frequency / divider;
}
/*----------------------------------------------------------------------------*/
static bool commonDividerReady(const void *clockBase)
{
  const struct CommonDividerClass * const clock = clockBase;
  const volatile uint32_t * const reg = calcDividerReg(clock->channel);

  return !(*reg & IDIV_PD);
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
static void pll0UsbClockDisable(const void *clockBase __attribute__((unused)))
{
  LPC_CGU->PLL0USB_CTRL |= BASE_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result pll0UsbClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct PllConfig * const config = configBase;

  assert(config->source != CLOCK_USB_PLL);
  assert(config->divisor);
  assert(config->multiplier/* && config->multiplier <= 65536*/); //FIXME

  const uint32_t sourceFrequency = getSourceFrequency(config->source);

  if (!sourceFrequency)
    return E_ERROR;

  unsigned int divisor = config->divisor;
  unsigned int psel = 1;

  //TODO Find common dividers
  while (psel < (1 << 5) && !(divisor & 1)) //TODO Constants
  {
    psel <<= 1;
    divisor >>= 1;
  }
  assert(divisor <= 256);

  const unsigned int msel = config->multiplier >> 1;
  const unsigned int nsel = divisor;
  bool directInput = false, directOutput = false;

  if (psel > 1)
    psel >>= 1;
  else
    directOutput = true;

  if (nsel == 1)
    directInput = true;

  const uint32_t ccoFrequency = sourceFrequency * config->multiplier;
  const uint32_t expectedFrequency = ccoFrequency / config->divisor;

  /* Check CCO range */
  assert(ccoFrequency >= 275000000 && ccoFrequency <= 550000000);

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

  LPC_CGU->PLL0USB_CTRL = BASE_PD;
  LPC_CGU->PLL0USB_MDIV = mDivValue;
  LPC_CGU->PLL0USB_NP_DIV = npDivValue;
  LPC_CGU->PLL0USB_CTRL |= controlValue;
  LPC_CGU->PLL0USB_CTRL &= ~BASE_PD;

  pll0UsbFrequency = expectedFrequency;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll0UsbClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  return pll0UsbFrequency;
}
/*----------------------------------------------------------------------------*/
static bool pll0UsbClockReady(const void *clockBase __attribute__((unused)))
{
  return pll0UsbFrequency && (LPC_CGU->PLL0USB_STAT & PLL0_STAT_LOCK);
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
  assert(config->divisor);
  assert(config->multiplier && config->multiplier <= 256);

  unsigned int divisor = config->divisor;
  unsigned int psel = 0;

  while (psel < 4 && !(divisor & 1))
  {
    ++psel;
    divisor >>= 1;
  }
  assert(divisor <= 4);

  const uint32_t sourceFrequency = getSourceFrequency(config->source);

  if (!sourceFrequency)
    return E_ERROR;

  const unsigned int msel = config->multiplier - 1;
  const unsigned int nsel = divisor - 1;
  bool direct = false;

  if (psel)
    --psel;
  else
    direct = true;

  const uint32_t ccoFrequency = sourceFrequency * config->multiplier;
  const uint32_t expectedFrequency = ccoFrequency / config->divisor;

  /* Check CCO range */
  assert(ccoFrequency >= 156000000 && ccoFrequency <= 320000000);

  uint32_t controlValue = BASE_AUTOBLOCK | BASE_CLK_SEL(config->source)
      | PLL1_CTRL_PSEL(psel) | PLL1_CTRL_NSEL(nsel) | PLL1_CTRL_MSEL(msel);

  if (expectedFrequency > 110000000)
  {
    /* No division allowed */
    assert(direct);

    /* Start at mid-range frequency by dividing output clock */
    LPC_CGU->PLL1_CTRL = BASE_PD;
    LPC_CGU->PLL1_CTRL |= controlValue;
    LPC_CGU->PLL1_CTRL &= ~BASE_PD;

    /* User manual recommends to add a delay for 50 microseconds */
    udelay(50);

    /* Check PLL state */
    if (!((LPC_CGU->PLL1_STAT & PLL1_STAT_LOCK)))
      return E_ERROR;

    /* Double the output frequency by enabling direct output */
    LPC_CGU->PLL1_CTRL |= PLL1_CTRL_DIRECT;
  }
  else
  {
    if (direct)
      controlValue |= PLL1_CTRL_FBSEL | PLL1_CTRL_DIRECT;

    LPC_CGU->PLL1_CTRL = BASE_PD;
    LPC_CGU->PLL1_CTRL |= controlValue;
    LPC_CGU->PLL1_CTRL &= ~BASE_PD;
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
  const struct CommonClockClass * const clock = clockBase;
  const struct ClockOutputConfig * const config = configBase;
  const struct CommonClockConfig baseConfig = {
      .source = config->source
  };

  const struct PinEntry * const pinEntry = pinFind(clockOutputPins,
      config->pin, clock->branch);
  assert(pinEntry);

  const struct Pin pin = pinInit(config->pin);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  return commonClockEnable(clock, &baseConfig);
}
/*----------------------------------------------------------------------------*/
static void commonClockDisable(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  volatile uint32_t * const reg = calcBranchReg(clock->branch);

  *reg |= BASE_PD;
}
/*----------------------------------------------------------------------------*/
static enum Result commonClockEnable(const void *clockBase,
    const void *configBase)
{
  const struct CommonClockClass * const clock = clockBase;
  const struct CommonClockConfig * const config = configBase;
  volatile uint32_t * const reg = calcBranchReg(clock->branch);

  if (clock->branch == CLOCK_BASE_M4)
    flashLatencyReset();

  *reg |= BASE_AUTOBLOCK;
  *reg = (*reg & ~BASE_CLK_SEL_MASK) | BASE_CLK_SEL(config->source);
  *reg &= ~BASE_PD;

  if (clock->branch == CLOCK_BASE_M4)
  {
    const uint32_t frequency = commonClockFrequency(clockBase);

    flashLatencyUpdate(frequency);
    ticksPerSecond = TICK_RATE(frequency);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t commonClockFrequency(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  const volatile uint32_t * const reg = calcBranchReg(clock->branch);

  return getSourceFrequency(BASE_CLK_SEL_VALUE(*reg));
}
/*----------------------------------------------------------------------------*/
static bool commonClockReady(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  const volatile uint32_t * const reg = calcBranchReg(clock->branch);

  return !(*reg & BASE_PD);
}
