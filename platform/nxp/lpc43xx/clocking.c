/*
 * clocking.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/platform_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/clocking_defs.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 12000000
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcBranchReg(enum clockBranch);
/*----------------------------------------------------------------------------*/
static enum result pll1ClockDisable(const void *);
static enum result pll1ClockEnable(const void *, const void *);
static uint32_t pll1ClockFrequency(const void *);
static bool pll1ClockReady(const void *);

static enum result commonClockDisable(const void *);
static enum result commonClockEnable(const void *, const void *);
static uint32_t commonClockFrequency(const void *);
static bool commonClockReady(const void *);
/*----------------------------------------------------------------------------*/
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
    .channel = CLOCK_BASE_M4
};

static const struct CommonClockClass usb0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_USB0
};

static const struct CommonClockClass usb1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_USB1
};

static const struct CommonClockClass peripheralClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_PERIPH
};

static const struct CommonClockClass apb1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_APB1
};

static const struct CommonClockClass apb3ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_APB3
};

static const struct CommonClockClass spifiClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_SPIFI
};

static const struct CommonClockClass spiClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_SPI
};

static const struct CommonClockClass phyRxClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_PHY_RX
};

static const struct CommonClockClass phyTxClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_PHY_TX
};

static const struct CommonClockClass lcdClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_LCD
};

static const struct CommonClockClass adcHsClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_ADCHS
};

static const struct CommonClockClass sdioClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_SDIO
};

static const struct CommonClockClass ssp0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_SSP0
};

static const struct CommonClockClass ssp1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_SSP1
};

static const struct CommonClockClass usart0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_USART0
};

static const struct CommonClockClass uart1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_UART1
};

static const struct CommonClockClass usart2ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_USART2
};

static const struct CommonClockClass usart3ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_USART3
};

static const struct CommonClockClass audioClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_OUT
};

static const struct CommonClockClass outClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_AUDIO
};

static const struct CommonClockClass cguOut0ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_CGU_OUT0
};

static const struct CommonClockClass cguOut1ClockTable = {
    .parent = {
        .disable = commonClockDisable,
        .enable = commonClockEnable,
        .frequency = commonClockFrequency,
        .ready = commonClockReady
    },
    .channel = CLOCK_BASE_CGU_OUT1
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const Pll1 = &pll1Table;
/*----------------------------------------------------------------------------*/
const struct CommonClockClass * const MainClock = &mainClockTable;
const struct CommonClockClass * const Usb0Clock = &usb0ClockTable;
const struct CommonClockClass * const Usb1Clock = &usb1ClockTable;
const struct CommonClockClass * const PeripheralClock = &peripheralClockTable;
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
uint32_t coreClock = INT_OSC_FREQUENCY; /* FIXME Used for SysTick only */
/*----------------------------------------------------------------------------*/
static volatile uint32_t *calcBranchReg(enum clockBranch source)
{
  return &LPC_CGU->BASE_USB0_CLK + source;
}
/*----------------------------------------------------------------------------*/
static enum result pll1ClockDisable(const void *clockBase
    __attribute__((unused)))
{
  //TODO Add current source checking
  LPC_CGU->PLL1_CTRL |= PLL1_CTRL_PD;
  return E_OK;
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
      //TODO Add checks
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
  LPC_CGU->PLL1_CTRL &= ~PLL1_CTRL_AUTOBLOCK; //TODO Remove

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
static enum result commonClockDisable(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  volatile uint32_t * const reg = calcBranchReg(clock->channel);

  *reg |= BASE_CLK_PD;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result commonClockEnable(const void *clockBase,
    const void *configBase)
{
  const struct CommonClockClass * const clock = clockBase;
  const struct CommonClockConfig * const config = configBase;
  volatile uint32_t * const reg = calcBranchReg(clock->channel);

  /* TODO Add configuration parameters checking */

  *reg |= BASE_CLK_AUTOBLOCK;
  *reg = (*reg & ~BASE_CLK_SEL_MASK) | BASE_CLK_SEL(config->source);
  *reg &= ~(BASE_CLK_PD | BASE_CLK_AUTOBLOCK); //FIXME Remove autoblock

  if (clock->channel == CLOCK_BASE_M4)
  {
    /* Update core clock frequency */
    coreClock = commonClockFrequency(clockBase);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t commonClockFrequency(const void *clockBase)
{
  const struct CommonClockClass * const clock = clockBase;
  volatile uint32_t * const reg = calcBranchReg(clock->channel);

  switch (BASE_CLK_SEL_VALUE(*reg))
  {
    case CLOCK_INTERNAL:
      return INT_OSC_FREQUENCY;

    case CLOCK_EXTERNAL:
      return extFrequency;

    case CLOCK_PLL0_USB:
      return pll0UsbFrequency;

    case CLOCK_PLL0_AUDIO:
      return pll0AudioFrequency;

    case CLOCK_PLL1:
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
  volatile uint32_t * const reg = calcBranchReg(clock->channel);

  return !(*reg & BASE_CLK_PD);
}
