/*
 * clocking.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <delay.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/clocking_defs.h>
#include <platform/nxp/lpc43xx/system.h>
#include <platform/nxp/lpc43xx/system_defs.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY     12000000
#define RTC_OSC_FREQUENCY     32768
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcBranchReg(enum clockBranch);
static inline volatile uint32_t *calcDividerReg(enum clockSource);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static uint32_t getSourceFrequency(enum clockSource);
/*----------------------------------------------------------------------------*/
static void commonDividerDisable(const void *);
static enum result commonDividerEnable(const void *, const void *);
static uint32_t commonDividerFrequency(const void *);
static bool commonDividerReady(const void *);

static void extOscDisable(const void *);
static enum result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static void rtcOscDisable(const void *);
static enum result rtcOscEnable(const void *, const void *);
static uint32_t rtcOscFrequency(const void *);
static bool rtcOscReady(const void *);

static void pll1ClockDisable(const void *);
static enum result pll1ClockEnable(const void *, const void *);
static uint32_t pll1ClockFrequency(const void *);
static bool pll1ClockReady(const void *);

static void commonClockDisable(const void *);
static enum result commonClockEnable(const void *, const void *);
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
    .branch = CLOCK_BASE_OUT
};

static const struct CommonClockClass outClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_AUDIO
};

static const struct CommonClockClass cguOut0ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT0
};

static const struct CommonClockClass cguOut1ClockTable = {
    .base = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT1
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
static inline volatile uint32_t *calcBranchReg(enum clockBranch branch)
{
  return &LPC_CGU->BASE_USB0_CLK + branch;
}
/*----------------------------------------------------------------------------*/
static inline volatile uint32_t *calcDividerReg(enum clockSource source)
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
  const uint32_t divisor = frequency / 43000000;
  const uint32_t remainder = frequency - divisor * 43000000;
  const uint8_t clocks = 1 + (divisor << 1) + (remainder >= 21000000 ? 1 : 0);

  sysFlashLatencyUpdate(clocks <= 10 ? clocks : 10);
}
/*----------------------------------------------------------------------------*/
static uint32_t getSourceFrequency(enum clockSource source)
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
static void commonDividerDisable(const void *clockBase)
{
  const struct CommonDividerClass * const clock = clockBase;
  volatile uint32_t * const reg = calcDividerReg(clock->channel);

  *reg |= IDIV_PD;
}
/*----------------------------------------------------------------------------*/
static enum result commonDividerEnable(const void *clockBase,
    const void *configBase)
{
  const struct CommonDividerClass * const clock = clockBase;
  const struct CommonDividerConfig * const config = configBase;
  volatile uint32_t * const reg = calcDividerReg(clock->channel);

  assert(config->value);
  assert(clock->channel != CLOCK_IDIVA || config->value <= 4);
  assert(clock->channel != CLOCK_IDIVE || config->value <= 256);
  assert((clock->channel == CLOCK_IDIVA || clock->channel == CLOCK_IDIVE)
      || config->value <= 16);

  *reg |= IDIV_AUTOBLOCK;
  *reg = (*reg & ~(IDIV_DIVIDER_MASK | IDIV_CLK_SEL_MASK))
      | IDIV_DIVIDER(config->value - 1) | IDIV_CLK_SEL(config->source);
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
static enum result extOscEnable(const void *clockBase __attribute__((unused)),
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
static enum result intOscEnable(const void *clockBase __attribute__((unused)),
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
static enum result rtcOscEnable(const void *clockBase __attribute__((unused)),
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
static void pll1ClockDisable(const void *clockBase __attribute__((unused)))
{
  LPC_CGU->PLL1_CTRL |= PLL1_CTRL_PD;
}
/*----------------------------------------------------------------------------*/
static enum result pll1ClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct PllConfig * const config = configBase;

  assert(config->source != CLOCK_PLL);
  assert(config->divisor);
  assert(config->multiplier && config->multiplier <= 256);

  uint8_t divisor = config->divisor;
  uint8_t psel = 0;

  while (psel < 4 && !(divisor & 1))
  {
    ++psel;
    divisor >>= 1;
  }
  assert(divisor <= 4);

  const uint32_t sourceFrequency = getSourceFrequency(config->source);

  if (!sourceFrequency)
    return E_ERROR;

  const uint8_t msel = config->multiplier - 1;
  const uint8_t nsel = divisor - 1;
  bool direct = false;

  if (psel)
    --psel;
  else
    direct = true;

  const uint32_t ccoFrequency = sourceFrequency * config->multiplier;
  const uint32_t expectedFrequency = ccoFrequency / config->divisor;

  /* Check CCO range */
  assert(ccoFrequency >= 156000000 && ccoFrequency <= 320000000);

  uint32_t controlValue = PLL1_CTRL_AUTOBLOCK | PLL1_CTRL_CLKSEL(config->source)
      | PLL1_CTRL_PSEL(psel) | PLL1_CTRL_NSEL(nsel) | PLL1_CTRL_MSEL(msel);

  if (expectedFrequency > 110000000)
  {
    /* No division allowed */
    assert(direct);

    /* Start at mid-range frequency by dividing output clock */
    LPC_CGU->PLL1_CTRL = PLL1_CTRL_PD;
    LPC_CGU->PLL1_CTRL |= controlValue;
    LPC_CGU->PLL1_CTRL &= ~PLL1_CTRL_PD;

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

    LPC_CGU->PLL1_CTRL = PLL1_CTRL_PD;
    LPC_CGU->PLL1_CTRL |= controlValue;
    LPC_CGU->PLL1_CTRL &= ~PLL1_CTRL_PD;
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
static void commonClockDisable(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  volatile uint32_t * const reg = calcBranchReg(clock->branch);

  *reg |= BASE_PD;
}
/*----------------------------------------------------------------------------*/
static enum result commonClockEnable(const void *clockBase,
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
