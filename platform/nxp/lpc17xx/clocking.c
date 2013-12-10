/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/platform_defs.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/clocking_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 4e6
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static void stubDisable(void);
static bool stubReady(void);
/*----------------------------------------------------------------------------*/
static void extOscDisable(void);
static enum result extOscEnable(const void *);
static bool extOscReady(void);
static void sysPllDisable(void);
static enum result sysPllEnable(const void *);
static bool sysPllReady(void);
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .ready = extOscReady
};

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = stubDisable,
    .enable = mainClockEnable,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass *ExternalOsc = &extOscTable;
const struct ClockClass *SystemPll = &sysPllTable;
const struct ClockClass *MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
static const uint32_t intOscFrequency = INT_OSC_FREQUENCY;
static uint32_t extOscFrequency = 0, rtcOscFrequency = 0, pllFrequency = 0;
static uint8_t pllDivider = 0;
uint32_t sysCoreClock = INT_OSC_FREQUENCY;
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Select safe setting */
  sysFlashLatency(6);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  uint8_t clocks = 1 + frequency / 20e6;

  if (clocks > 5)
    clocks = 5;
  sysFlashLatency(clocks);
}
/*----------------------------------------------------------------------------*/
static void stubDisable(void)
{

}
/*----------------------------------------------------------------------------*/
static bool stubReady(void)
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(void)
{
  LPC_SC->SCS &= ~SCS_OSCEN;
}
/*----------------------------------------------------------------------------*/
static enum result extOscEnable(const void *configPtr)
{
  const struct ExternalOscConfig * const config = configPtr;
  uint32_t buffer = LPC_SC->SCS | SCS_OSCEN;

  /*
   * Oscillator can operate in two modes: slave mode and oscillation mode.
   * Slave mode does not require any additional configuration.
   */

  if (config->frequency > 15e6)
    buffer |= SCS_FREQRANGE;

  extOscFrequency = config->frequency;
  LPC_SC->SCS = buffer;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(void)
{
  return LPC_SC->SCS & SCS_OSCSTAT ? true : false;
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(void)
{
  //TODO
}
/*----------------------------------------------------------------------------*/
static enum result sysPllEnable(const void *configPtr)
{
  const struct PllConfig * const config = configPtr;
  uint32_t frequency; /* Resulting CCO frequency */
  uint32_t source; /* Clock Source Select register value */
  uint16_t multiplier;
  uint8_t prescaler;

  assert(config->multiplier && config->divider);

  /*
   * Calculations:
   * multiplier = (F_CCO * prescaler) / (2 * F_IN)
   * output = F_CCO / postscaler
   */

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured */
      if (!extOscFrequency)
        return E_ERROR;
      source = CLKSRCSEL_MAIN;
      frequency = extOscFrequency;
      break;
    case CLOCK_INTERNAL:
      source = CLKSRCSEL_IRC;
      frequency = intOscFrequency;
      break;
    case CLOCK_RTC:
      if (!rtcOscFrequency)
        return E_ERROR;
      source = CLKSRCSEL_RTC;
      frequency = rtcOscFrequency;
      break;
    default:
      return E_ERROR;
  }

  multiplier = config->multiplier;
  frequency = frequency * config->multiplier;
  if (config->source != CLOCK_RTC)
  {
    if (multiplier < 6 || multiplier > 512)
      return E_VALUE;

    if (frequency < 275e6 || frequency > 550e6)
      return E_ERROR;

    prescaler = 2;
  }
  else
  {
    /* Low-frequency source supports only a limited set of multiplier values */
    /* No check is performed due to complexity */
    prescaler = 1 + frequency / 550e6;
  }

  /* Update system frequency and postscaler value */
  pllFrequency = frequency / config->divider;
  pllDivider = config->divider - 1;

  /* Set clock source */
  LPC_SC->CLKSRCSEL = source;

  /* Update PLL clock source */
  LPC_SC->PLL0CFG = PLL0CFG_MSEL(multiplier - 1) | PLL0CFG_NSEL(prescaler - 1);
  LPC_SC->PLL0FEED = 0xAA;
  LPC_SC->PLL0FEED = 0x55;

  /* Enable PLL */
  LPC_SC->PLL0CON = PLL0CON_ENABLE;
  LPC_SC->PLL0FEED  = 0xAA;
  LPC_SC->PLL0FEED  = 0x55;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(void)
{
  return LPC_SC->PLL0STAT & PLL0STAT_LOCK ? true : false;
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *configPtr)
{
  const struct MainClockConfig * const config = configPtr;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      flashLatencyReset();
      LPC_SC->CLKSRCSEL = CLKSRCSEL_IRC;
      sysCoreClock = intOscFrequency;
    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured */
      if (!extOscFrequency)
        return E_ERROR;
      flashLatencyReset();
      LPC_SC->CLKSRCSEL = CLKSRCSEL_MAIN;
      sysCoreClock = extOscFrequency;
      break;
    case CLOCK_PLL:
      /* Check whether PLL is configured */
      if (!pllFrequency)
        return E_ERROR;

      /* Maximize flash latency and configure system clock divider */
      flashLatencyReset();
      LPC_SC->CCLKCFG = pllDivider;

      /* Connect PLL */
      LPC_SC->PLL0CON |= PLL0CON_CONNECT;
      LPC_SC->PLL0FEED = 0xAA;
      LPC_SC->PLL0FEED = 0x55;

      /* Wait for enable and connect */
      while (!(LPC_SC->PLL0STAT & (PLL0STAT_ENABLED | PLL0STAT_CONNECTED)));

      sysCoreClock = pllFrequency;
      break;
    case CLOCK_RTC:
      if (!rtcOscFrequency)
        return E_ERROR;
      flashLatencyReset();
      LPC_SC->CLKSRCSEL = CLKSRCSEL_RTC;
      sysCoreClock = rtcOscFrequency;
      break;
    default:
      return E_ERROR;
  }
  flashLatencyUpdate(sysCoreClock);

  return E_OK;
}
