/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/clocking_defs.h>
#include <platform/nxp/lpc17xx/system.h>
#include <platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY     4000000
#define RTC_OSC_FREQUENCY     32768
#define USB_FREQUENCY         48000000
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void);
static void flashLatencyUpdate(uint32_t);
static void pllDisconnect(void);

static void clockDisableStub(const void *);
static enum result clockEnableStub(const void *, const void *);
static bool clockReadyStub(const void *);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static uint32_t intOscFrequency(const void *);

static uint32_t rtcOscFrequency(const void *);

static void sysPllDisable(const void *);
static enum result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);

static void usbPllDisable(const void *);
static enum result usbPllEnable(const void *, const void *);
static uint32_t usbPllFrequency(const void *);
static bool usbPllReady(const void *);
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);

static enum result usbClockEnable(const void *, const void *);
static uint32_t usbClockFrequency(const void *);
static bool usbClockReady(const void *);
/*----------------------------------------------------------------------------*/
static const struct ClockClass extOscTable = {
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

static const struct ClockClass intOscTable = {
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = intOscFrequency,
    .ready = clockReadyStub
};

static const struct ClockClass rtcOscTable = {
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = rtcOscFrequency,
    .ready = clockReadyStub
};

static const struct ClockClass sysPllTable = {
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};

static const struct ClockClass usbPllTable = {
    .disable = usbPllDisable,
    .enable = usbPllEnable,
    .frequency = usbPllFrequency,
    .ready = usbPllReady
};
/*----------------------------------------------------------------------------*/
static const struct ClockClass mainClockTable = {
    .disable = clockDisableStub,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = clockReadyStub
};

static const struct ClockClass usbClockTable = {
    .disable = clockDisableStub,
    .enable = usbClockEnable,
    .frequency = usbClockFrequency,
    .ready = usbClockReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &extOscTable;
const struct ClockClass * const InternalOsc = &intOscTable;
const struct ClockClass * const RtcOsc = &rtcOscTable;
const struct ClockClass * const SystemPll = &sysPllTable;
const struct ClockClass * const UsbPll = &usbPllTable;
const struct ClockClass * const MainClock = &mainClockTable;
const struct ClockClass * const UsbClock = &usbClockTable;
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
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
static void clockDisableStub(const void *clockBase __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result clockEnableStub(const void *clockBase
    __attribute__((unused)), const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static bool clockReadyStub(const void *clockBase __attribute__((unused)))
{
  return true;
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

  assert(config->frequency >= 1000000 && config->frequency <= 25000000);

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
static uint32_t intOscFrequency(const void *clockBase __attribute__((unused)))
{
  return INT_OSC_FREQUENCY;
}
/*----------------------------------------------------------------------------*/
static uint32_t rtcOscFrequency(const void *clockBase __attribute__((unused)))
{
  return RTC_OSC_FREQUENCY;
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
  uint32_t frequency = 0; /* Resulting CCO frequency */
  uint32_t source = 0; /* Clock Source Select register value */
  uint16_t multiplier;

  assert(config->multiplier);
  assert(config->divisor);

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
      frequency = RTC_OSC_FREQUENCY;
      break;

    default:
      break;
  }
  assert(frequency);

  multiplier = config->multiplier;
  frequency = frequency * config->multiplier;

  uint8_t prescaler;

  if (config->source != CLOCK_RTC)
  {
    assert(multiplier >= 6 && multiplier <= 512);
    assert(frequency >= 275000000 && frequency <= 550000000);

    prescaler = 2;
  }
  else
  {
    /* Low-frequency source supports only a limited set of multiplier values */
    /* No check is performed due to complexity */
    prescaler = 1 + frequency / 550000000;
  }

  /* Update system frequency and postscaler value */
  pllFrequency = frequency;
  pllDivisor = config->divisor;

  /* Set clock source */
  LPC_SC->CLKSRCSEL = source;

  /* Update PLL clock source */
  LPC_SC->PLL0CFG = PLL0CFG_MSEL(multiplier - 1) | PLL0CFG_NSEL(prescaler - 1);
  LPC_SC->PLL0FEED = PLLFEED_FIRST;
  LPC_SC->PLL0FEED = PLLFEED_SECOND;

  /* Enable PLL */
  LPC_SC->PLL0CON = PLL0CON_ENABLE;
  LPC_SC->PLL0FEED = PLLFEED_FIRST;
  LPC_SC->PLL0FEED = PLLFEED_SECOND;

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
static void usbPllDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SC->PLL1CON &= ~PLL1CON_ENABLE;
  LPC_SC->PLL1FEED = PLLFEED_FIRST;
  LPC_SC->PLL1FEED = PLLFEED_SECOND;
}
/*----------------------------------------------------------------------------*/
static enum result usbPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;

  assert(config->multiplier);
  assert(config->divisor);
  assert(config->source == CLOCK_EXTERNAL);

  /*
   * Calculations:
   * MSEL = USBCLK / F_OSC
   * PSEL = F_CCO / (USBCLK * 2)
   */

  /* Check CCO frequency */
  assert(extFrequency * config->multiplier >= 156000000
      && extFrequency * config->multiplier <= 320000000);

  const uint8_t msel = USB_FREQUENCY / extFrequency - 1;
  uint8_t psel = 0;
  uint8_t sourceDivisor = config->divisor >> 1;

  while (psel < 4 && sourceDivisor != 1 << psel)
    psel++;
  /* Check whether actual divisor value found */
  assert(psel != 4);

  /* Update PLL clock source */
  LPC_SC->PLL1CFG = PLL1CFG_MSEL(msel) | PLL1CFG_PSEL(psel);
  LPC_SC->PLL1FEED = PLLFEED_FIRST;
  LPC_SC->PLL1FEED = PLLFEED_SECOND;

  /* Enable PLL */
  LPC_SC->PLL1CON = PLL1CON_ENABLE;
  LPC_SC->PLL1FEED = PLLFEED_FIRST;
  LPC_SC->PLL1FEED = PLLFEED_SECOND;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbPllFrequency(const void *clockBase __attribute__((unused)))
{
  return LPC_SC->PLL1STAT & PLL1STAT_LOCK ? USB_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool usbPllReady(const void *clockBase __attribute__((unused)))
{
  return (LPC_SC->PLL1STAT & PLL1STAT_LOCK) != 0;
}
/*----------------------------------------------------------------------------*/
static enum result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct CommonClockConfig * const config = configBase;

  if (config->source != CLOCK_PLL)
  {
    assert(config->source == CLOCK_INTERNAL
        || config->source == CLOCK_EXTERNAL
        || config->source == CLOCK_RTC);

    uint32_t source = 0;

    switch (config->source)
    {
      case CLOCK_INTERNAL:
        source = CLKSRCSEL_IRC;
        break;

      case CLOCK_EXTERNAL:
        source = CLKSRCSEL_MAIN;
        break;

      case CLOCK_RTC:
        source = CLKSRCSEL_RTC;
        break;

      default:
        break;
    }

    if (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED)
      pllDisconnect();

    LPC_SC->CLKSRCSEL = (uint32_t)source;
  }
  else
  {
    LPC_SC->CCLKCFG = pllDivisor - 1;

    /* Connect PLL */
    LPC_SC->PLL0CON |= PLL0CON_CONNECT;
    LPC_SC->PLL0FEED = PLLFEED_FIRST;
    LPC_SC->PLL0FEED = PLLFEED_SECOND;

    /* Wait for enable and connect */
    const uint32_t mask = PLL0STAT_ENABLED | PLL0STAT_CONNECTED;
    while ((LPC_SC->PLL0STAT & mask) != mask);
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
        return pllFrequency / pllDivisor;
      else
        return extFrequency;

    case CLKSRCSEL_RTC:
      return RTC_OSC_FREQUENCY;

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum result usbClockEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct CommonClockConfig * const config = configBase;

  assert(config->source == CLOCK_PLL || config->source == CLOCK_USB_PLL);

  switch (config->source)
  {
    case CLOCK_PLL:
    {
      const uint8_t divisor = pllFrequency / 48000000 - 1;
      assert(divisor == 5 || divisor == 7 || divisor == 9);

      LPC_SC->USBCLKCFG = USBCLKCFG_USBSEL(divisor);
      break;
    }

    case CLOCK_USB_PLL:
    {
      /* Connect PLL */
      LPC_SC->PLL1CON |= PLL1CON_CONNECT;
      LPC_SC->PLL1FEED = PLLFEED_FIRST;
      LPC_SC->PLL1FEED = PLLFEED_SECOND;

      /* Wait for enable and connect */
      const uint32_t mask = PLL1STAT_ENABLED | PLL1STAT_CONNECTED;
      while ((LPC_SC->PLL1STAT & mask) != mask);

      break;
    }

    default:
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t usbClockFrequency(const void *configBase)
{
  return usbClockReady(configBase) ? USB_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool usbClockReady(const void *configBase __attribute__((unused)))
{
  const uint8_t actualDivisor = USBCLKCFG_USBSEL_VALUE(LPC_SC->USBCLKCFG);
  const uint8_t divisor = pllFrequency / 48000000 - 1;

  return (actualDivisor && actualDivisor == divisor)
      || (LPC_SC->PLL1STAT & PLL1STAT_LOCK);
}
