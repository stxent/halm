/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/platform_defs.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/clocking_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 4000000
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static void pllDisconnect(void);
static enum result stubDisable(void);
static bool stubReady(void);
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(void);
static enum result extOscEnable(const void *);
static uint32_t extOscFrequency(void);
static bool extOscReady(void);

static enum result sysPllDisable(void);
static enum result sysPllEnable(const void *);
static uint32_t sysPllFrequency(void);
static bool sysPllReady(void);
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *);
static uint32_t mainClockFrequency(void);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = stubDisable,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = stubReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct ClockClass * const MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
static uint32_t rtcFrequency = 0;
static uint8_t pllDivider = 0;
uint32_t coreClock = INT_OSC_FREQUENCY;
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Select safe setting */
  sysFlashLatency(6);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  uint8_t clocks = 1 + frequency / 20000000;

  if (clocks > 5)
    clocks = 5;
  sysFlashLatency(clocks);
}
/*----------------------------------------------------------------------------*/
static void pllDisconnect(void)
{
  LPC_SC->PLL0CON &= ~PLL0CON_CONNECT;
  LPC_SC->PLL0FEED = PLLFEED_FIRST;
  LPC_SC->PLL0FEED = PLLFEED_SECOND;
}
/*----------------------------------------------------------------------------*/
static enum result stubDisable(void)
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static bool stubReady(void)
{
  return true;
}
/*----------------------------------------------------------------------------*/
static enum result extOscDisable(void)
{
  LPC_SC->SCS &= ~SCS_OSCEN;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result extOscEnable(const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  uint32_t buffer = LPC_SC->SCS | SCS_OSCEN;

  /*
   * Oscillator can operate in two modes: slave mode and oscillation mode.
   * Slave mode does not require any additional configuration.
   */

  if (config->frequency > 15000000)
    buffer |= SCS_FREQRANGE;

  extFrequency = config->frequency;
  LPC_SC->SCS = buffer;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(void)
{
  return extFrequency;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(void)
{
  return extFrequency && (LPC_SC->SCS & SCS_OSCSTAT ? true : false);
}
/*----------------------------------------------------------------------------*/
static enum result sysPllDisable(void)
{
  if (!(LPC_SC->PLL0STAT & PLL0STAT_CONNECTED))
  {
    LPC_SC->PLL0CON &= ~PLL0CON_ENABLE;
    LPC_SC->PLL0FEED = PLLFEED_FIRST;
    LPC_SC->PLL0FEED = PLLFEED_SECOND;
    return E_OK;
  }
  else
    return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result sysPllEnable(const void *configBase)
{
  const struct PllConfig * const config = configBase;
  uint32_t frequency; /* Resulting CCO frequency */
  uint32_t source; /* Clock Source Select register value */
  uint16_t multiplier;
  uint8_t prescaler;

  if (!config->multiplier || !config->divider)
    return E_VALUE;

  /*
   * Calculations:
   * multiplier = (F_CCO * prescaler) / (2 * F_IN)
   * output = F_CCO / postscaler
   */

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured */
      if (!extOscReady())
        return E_ERROR;
      source = CLKSRCSEL_MAIN;
      frequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      source = CLKSRCSEL_IRC;
      frequency = INT_OSC_FREQUENCY;
      break;

    case CLOCK_RTC:
      if (!rtcFrequency) //TODO Replace with rtcOscReady
        return E_ERROR;
      source = CLKSRCSEL_RTC;
      frequency = rtcFrequency;
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

    if (frequency < 275000000 || frequency > 550000000)
      return E_ERROR;

    prescaler = 2;
  }
  else
  {
    /* Low-frequency source supports only a limited set of multiplier values */
    /* No check is performed due to complexity */
    prescaler = 1 + frequency / 550000000;
  }

  /* Update system frequency and postscaler value */
  pllFrequency = frequency / config->divider;
  pllDivider = config->divider - 1;

  /* Set clock source */
  LPC_SC->CLKSRCSEL = source;

  /* Update PLL clock source */
  LPC_SC->PLL0CFG = PLL0CFG_MSEL(multiplier - 1) | PLL0CFG_NSEL(prescaler - 1);
  LPC_SC->PLL0FEED = PLLFEED_FIRST;
  LPC_SC->PLL0FEED = PLLFEED_SECOND;

  /* Enable PLL */
  LPC_SC->PLL0CON = PLL0CON_ENABLE;
  LPC_SC->PLL0FEED  = PLLFEED_FIRST;
  LPC_SC->PLL0FEED  = PLLFEED_SECOND;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t sysPllFrequency(void)
{
  return pllFrequency;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(void)
{
  return pllFrequency && (LPC_SC->PLL0STAT & PLL0STAT_LOCK ? true : false);
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *configBase)
{
  const struct MainClockConfig * const config = configBase;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      flashLatencyReset();
      if (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED)
        pllDisconnect();
      LPC_SC->CLKSRCSEL = CLKSRCSEL_IRC;
      coreClock = INT_OSC_FREQUENCY;

    case CLOCK_EXTERNAL:
      /* Check whether external oscillator is configured and ready */
      if (!extOscReady())
        return E_ERROR;

      flashLatencyReset();
      if (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED)
        pllDisconnect();
      LPC_SC->CLKSRCSEL = CLKSRCSEL_MAIN;
      coreClock = extFrequency;
      break;

    case CLOCK_PLL:
      /* Check whether PLL is configured and ready */
      if (!sysPllReady())
        return E_ERROR;

      /* Maximize flash latency and configure system clock divider */
      flashLatencyReset();
      LPC_SC->CCLKCFG = pllDivider;

      /* Connect PLL */
      LPC_SC->PLL0CON |= PLL0CON_CONNECT;
      LPC_SC->PLL0FEED = PLLFEED_FIRST;
      LPC_SC->PLL0FEED = PLLFEED_SECOND;

      /* Wait for enable and connect */
      while (!(LPC_SC->PLL0STAT & (PLL0STAT_ENABLED | PLL0STAT_CONNECTED)));

      coreClock = pllFrequency;
      break;

    case CLOCK_RTC:
      if (!rtcFrequency) //TODO Replace with rtcOscReady
        return E_ERROR;

      flashLatencyReset();
      LPC_SC->CLKSRCSEL = CLKSRCSEL_RTC;
      coreClock = rtcFrequency;
      break;

    default:
      return E_ERROR;
  }
  flashLatencyUpdate(coreClock);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(void)
{
  return coreClock;
}
