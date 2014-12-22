/*
 * clocking.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <delay.h>
#include <platform/platform_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/clocking_defs.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY               12000000
#define TICK_RATE(frequency)            ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcBranchReg(enum clockBranch);
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void pll1ClockDisable(const void *);
static enum result pll1ClockEnable(const void *, const void *);
static uint32_t pll1ClockFrequency(const void *);
static bool pll1ClockReady(const void *);

static void commonClockDisable(const void *);
static enum result commonClockEnable(const void *, const void *);
static uint32_t commonClockFrequency(const void *);
static bool commonClockReady(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

static const struct ClockClass pll1Table = {
    .disable = pll1ClockDisable,
    .enable = pll1ClockEnable,
    .frequency = pll1ClockFrequency,
    .ready = pll1ClockReady
};
/*----------------------------------------------------------------------------*/
static const struct CommonClockClass mainClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_M4
};

static const struct CommonClockClass usb0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USB0
};

static const struct CommonClockClass usb1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USB1
};

static const struct CommonClockClass periphClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_PERIPH
};

static const struct CommonClockClass apb1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_APB1
};

static const struct CommonClockClass apb3ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_APB3
};

static const struct CommonClockClass spifiClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SPIFI
};

static const struct CommonClockClass spiClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SPI
};

static const struct CommonClockClass phyRxClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_PHY_RX
};

static const struct CommonClockClass phyTxClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_PHY_TX
};

static const struct CommonClockClass lcdClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_LCD
};

static const struct CommonClockClass adcHsClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_ADCHS
};

static const struct CommonClockClass sdioClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SDIO
};

static const struct CommonClockClass ssp0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SSP0
};

static const struct CommonClockClass ssp1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_SSP1
};

static const struct CommonClockClass usart0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USART0
};

static const struct CommonClockClass uart1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_UART1
};

static const struct CommonClockClass usart2ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USART2
};

static const struct CommonClockClass usart3ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_USART3
};

static const struct CommonClockClass audioClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_OUT
};

static const struct CommonClockClass outClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_AUDIO
};

static const struct CommonClockClass cguOut0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT0
};

static const struct CommonClockClass cguOut1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .branch = CLOCK_BASE_CGU_OUT1
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
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
static volatile uint32_t *calcBranchReg(enum clockBranch source)
{
  return &LPC_CGU->BASE_USB0_CLK + source;
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
static void pll1ClockDisable(const void *clockBase __attribute__((unused)))
{
  LPC_CGU->PLL1_CTRL |= PLL1_CTRL_PD;
}
/*----------------------------------------------------------------------------*/
static enum result pll1ClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct PllConfig * const config = configBase;
  uint32_t frequency;
  uint8_t msel, nsel;

  if (!config->multiplier || !config->divider)
    return E_VALUE;

  if (config->multiplier > 256 || config->divider > 4)
    return E_VALUE;

  msel = config->multiplier - 1;
  nsel = config->divider - 1;

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      frequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      frequency = INT_OSC_FREQUENCY;
      break;

    default:
      return E_ERROR;
  }

  /* Check CCO range */
  frequency = frequency * config->multiplier;
  if (frequency < 156000000 || frequency > 320000000)
    return E_ERROR;
  pll1Frequency = frequency / config->divider;

  LPC_CGU->PLL1_CTRL = PLL1_CTRL_PD;
  LPC_CGU->PLL1_CTRL |= PLL1_CTRL_FBSEL | PLL1_CTRL_DIRECT | PLL1_CTRL_AUTOBLOCK
      | PLL1_CTRL_PSEL(0) | PLL1_CTRL_NSEL(nsel) | PLL1_CTRL_MSEL(msel)
      | PLL1_CTRL_CLKSEL(config->source);
  LPC_CGU->PLL1_CTRL &= ~PLL1_CTRL_PD;

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

  *reg |= BASE_CLK_PD;
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

  *reg |= BASE_CLK_AUTOBLOCK;
  *reg = (*reg & ~BASE_CLK_SEL_MASK) | BASE_CLK_SEL(config->source);
  *reg &= ~BASE_CLK_PD;

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

  switch (BASE_CLK_SEL_VALUE(*reg))
  {
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

    default:
      /* Unknown clock source */
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static bool commonClockReady(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  const volatile uint32_t * const reg = calcBranchReg(clock->branch);

  return !(*reg & BASE_CLK_PD);
}
