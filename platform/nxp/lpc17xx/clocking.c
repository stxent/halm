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
#define INT_OSC_FREQUENCY               4000000
#define TICK_RATE(frequency)            ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static void pllDisconnect(void);
/*----------------------------------------------------------------------------*/
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

static void sysPllDisable(const void *);
static enum result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);
/*----------------------------------------------------------------------------*/
static void mainClockDisable(const void *);
static enum result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
static bool mainClockReady(const void *);
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

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = mainClockDisable,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = mainClockReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const RtcOsc = &rtcOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct ClockClass * const MainClock = &mainClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
static uint32_t rtcFrequency = 0;
static uint8_t pllDivisor = 0;
uint32_t ticksPerSecond = TICK_RATE(INT_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Select safe setting */
  sysFlashLatency(6);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  const uint8_t clocks = 1 + frequency / 20000000;

  sysFlashLatency(clocks <= 5 ? clocks : 5);
}
/*----------------------------------------------------------------------------*/
static void pllDisconnect(void)
{
  LPC_SC->PLL0CON &= ~PLL0CON_CONNECT;
  LPC_SC->PLL0FEED = PLLFEED_FIRST;
  LPC_SC->PLL0FEED = PLLFEED_SECOND;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SC->SCS &= ~SCS_OSCEN;
}
/*----------------------------------------------------------------------------*/
static enum result extOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
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
static uint32_t extOscFrequency(const void *clockBase __attribute__((unused)))
{
  return extFrequency;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *clockBase __attribute__((unused)))
{
  return extFrequency && (LPC_SC->SCS & SCS_OSCSTAT);
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

}
/*----------------------------------------------------------------------------*/
static enum result rtcOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static uint32_t rtcOscFrequency(const void *clockBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static bool rtcOscReady(const void *clockBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SC->PLL0CON &= ~PLL0CON_ENABLE;
  LPC_SC->PLL0FEED = PLLFEED_FIRST;
  LPC_SC->PLL0FEED = PLLFEED_SECOND;
}
/*----------------------------------------------------------------------------*/
static enum result sysPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;
  uint32_t frequency; /* Resulting CCO frequency */
  uint32_t source; /* Clock Source Select register value */
  uint16_t multiplier;
  uint8_t prescaler;

  if (!config->multiplier || !config->divisor)
    return E_VALUE;

  /*
   * Calculations:
   * multiplier = (F_CCO * prescaler) / (2 * F_IN)
   * output = F_CCO / postscaler
   */

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      source = CLKSRCSEL_MAIN;
      frequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      source = CLKSRCSEL_IRC;
      frequency = INT_OSC_FREQUENCY;
      break;

    case CLOCK_RTC:
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
  pllFrequency = frequency / config->divisor;
  pllDivisor = config->divisor - 1;

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
static uint32_t sysPllFrequency(const void *clockBase __attribute__((unused)))
{
  return pllFrequency;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(const void *clockBase __attribute__((unused)))
{
  return pllFrequency && (LPC_SC->PLL0STAT & PLL0STAT_LOCK);
}
/*----------------------------------------------------------------------------*/
static void mainClockDisable(const void *clockBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct MainClockConfig * const config = configBase;
  int8_t source = -1;

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      source = CLKSRCSEL_IRC;
      break;

    case CLOCK_EXTERNAL:
      source = CLKSRCSEL_MAIN;
      break;

    case CLOCK_PLL:
      break;

    case CLOCK_RTC:
      source = CLKSRCSEL_RTC;
      break;

    default:
      return E_ERROR;
  }

  if (source != -1)
  {
    if (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED)
      pllDisconnect();

    LPC_SC->CLKSRCSEL = (uint32_t)source;
  }
  else
  {
    LPC_SC->CCLKCFG = pllDivisor;

    /* Connect PLL */
    LPC_SC->PLL0CON |= PLL0CON_CONNECT;
    LPC_SC->PLL0FEED = PLLFEED_FIRST;
    LPC_SC->PLL0FEED = PLLFEED_SECOND;

    /* Wait for enable and connect */
    while (!(LPC_SC->PLL0STAT & (PLL0STAT_ENABLED | PLL0STAT_CONNECTED)));
  }

  const uint32_t frequency = mainClockFrequency(MainClock);

  flashLatencyUpdate(frequency);
  ticksPerSecond = TICK_RATE(frequency);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  switch (LPC_SC->CLKSRCSEL)
  {
    case CLKSRCSEL_IRC:
      return INT_OSC_FREQUENCY;

    case CLKSRCSEL_MAIN:
      if (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED)
        return pllFrequency;
      else
        return extFrequency;

    case CLKSRCSEL_RTC:
      return rtcFrequency;

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static bool mainClockReady(const void *clockBase __attribute__((unused)))
{
  return true;
}
