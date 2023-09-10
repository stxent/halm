/*
 * clocking.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/lpc17xx/clocking_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
#include <assert.h>
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
static enum Result clockEnableStub(const void *, const void *);
static bool clockReadyStub(const void *);
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static uint32_t intOscFrequency(const void *);

static uint32_t rtcOscFrequency(const void *);

static void sysPllDisable(const void *);
static enum Result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);

static void usbPllDisable(const void *);
static enum Result usbPllEnable(const void *, const void *);
static uint32_t usbPllFrequency(const void *);
static bool usbPllReady(const void *);
/*----------------------------------------------------------------------------*/
static void clockOutputDisable(const void *);
static enum Result clockOutputEnable(const void *, const void *);
static uint32_t clockOutputFrequency(const void *);
static bool clockOutputReady(const void *);

static enum Result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);

static enum Result usbClockEnable(const void *, const void *);
static uint32_t usbClockFrequency(const void *);
static bool usbClockReady(const void *);
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &(const struct ClockClass){
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

const struct ClockClass * const InternalOsc = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = intOscFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const RtcOsc = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = clockEnableStub,
    .frequency = rtcOscFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};

const struct ClockClass * const UsbPll = &(const struct ClockClass){
    .disable = usbPllDisable,
    .enable = usbPllEnable,
    .frequency = usbPllFrequency,
    .ready = usbPllReady
};
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ClockOutput = &(const struct ClockClass){
    .disable = clockOutputDisable,
    .enable = clockOutputEnable,
    .frequency = clockOutputFrequency,
    .ready = clockOutputReady
};

const struct ClockClass * const MainClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const UsbClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = usbClockEnable,
    .frequency = usbClockFrequency,
    .ready = usbClockReady
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry clockOutputPins[] = {
    {
        .key = PIN(1, 27), /* CLKOUT */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
static uint32_t pllFrequency = 0;
static uint8_t pllDivisor = 0;
uint32_t ticksPerSecond = TICK_RATE(INT_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static inline void flashLatencyReset(void)
{
  /* Set safe latency settings */
  sysFlashLatencyUpdate(6);
}
/*----------------------------------------------------------------------------*/
static void flashLatencyUpdate(uint32_t frequency)
{
  static const uint32_t frequencyStep = 20000000;
  const unsigned int clocks = (frequency + (frequencyStep - 1)) / frequencyStep;

  sysFlashLatencyUpdate(MIN(clocks, 5));
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
static enum Result clockEnableStub(const void *clockBase
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
static enum Result extOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  assert(config != NULL);
  assert(config->frequency >= 1000000 && config->frequency <= 25000000);

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
  return (LPC_SC->SCS & SCS_OSCSTAT) ? extFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *clockBase __attribute__((unused)))
{
  return extFrequency && (LPC_SC->SCS & SCS_OSCSTAT) != 0;
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
static enum Result sysPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);

  if (!config->multiplier || !config->divisor)
    return E_VALUE;

  uint32_t ccoFrequency = 0;
  uint32_t clockSource = 0;

  switch (config->source)
  {
    case CLOCK_EXTERNAL:
      clockSource = CLKSRCSEL_MAIN;
      ccoFrequency = extFrequency;
      break;

    case CLOCK_INTERNAL:
      clockSource = CLKSRCSEL_IRC;
      ccoFrequency = INT_OSC_FREQUENCY;
      break;

    case CLOCK_RTC:
      clockSource = CLKSRCSEL_RTC;
      ccoFrequency = RTC_OSC_FREQUENCY;
      break;

    default:
      break;
  }

  if (!ccoFrequency)
    return E_IDLE;

  /*
   * Calculations:
   * multiplier = (F_CCO * prescaler) / (2 * F_IN)
   * output = F_CCO / postscaler
   */

  unsigned int msel = config->multiplier;
  unsigned int nsel;

  if (config->source != CLOCK_RTC)
  {
    if (msel < 6 || msel > 512)
      return E_VALUE;

    ccoFrequency *= msel;
    nsel = 2;
  }
  else
  {
    /*
     * Low-frequency source supports only a limited set of multiplier values.
     * No check is performed due to complexity.
     */

    ccoFrequency *= msel * 2;
    nsel = 1 + ccoFrequency / 550000000;
  }

  /* Check CCO ccoFrequency */
  if (ccoFrequency < 275000000 || ccoFrequency > 550000000)
    return E_VALUE;

  /* Update system ccoFrequency and postscaler value */
  pllFrequency = ccoFrequency;
  pllDivisor = config->divisor;

  /* Set clock source */
  LPC_SC->CLKSRCSEL = clockSource;

  /* Update PLL clock source */
  LPC_SC->PLL0CFG = PLL0CFG_MSEL(msel - 1) | PLL0CFG_NSEL(nsel - 1);
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
  return (LPC_SC->PLL0STAT & PLL0STAT_LOCK) ? pllFrequency : 0;
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
static enum Result usbPllEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);

  if (!config->multiplier || !config->divisor)
    return E_VALUE;
  if (config->source != CLOCK_EXTERNAL)
    return E_VALUE;
  if (!extFrequency)
    return E_IDLE;

  /*
   * Calculations:
   * MSEL = USBCLK / F_OSC
   * PSEL = F_CCO / (USBCLK * 2)
   */

  const unsigned int msel = USB_FREQUENCY / extFrequency;
  const unsigned int psel = 31 - countLeadingZeros32(config->divisor);
  const uint32_t ccoFrequency = (extFrequency * msel) << psel;

  if (msel > 32 || psel > 4)
    return E_VALUE;
  if (1 << psel != config->divisor || msel << psel != config->multiplier)
    return E_VALUE;
  if (ccoFrequency < 156000000 || ccoFrequency > 320000000)
    return E_VALUE;

  /* Update PLL clock source */
  LPC_SC->PLL1CFG = PLL1CFG_MSEL(msel - 1) | PLL1CFG_PSEL(psel - 1);
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
  return (LPC_SC->PLL1STAT & PLL1STAT_LOCK) ? USB_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool usbPllReady(const void *clockBase __attribute__((unused)))
{
  return (LPC_SC->PLL1STAT & PLL1STAT_LOCK) != 0;
}
/*----------------------------------------------------------------------------*/
static void clockOutputDisable(const void *clockBase __attribute__((unused)))
{
  LPC_SC->CLKOUTCFG &= ~CLKOUTCFG_EN;
}
/*----------------------------------------------------------------------------*/
static enum Result clockOutputEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct ClockOutputConfig * const config = configBase;
  assert(config != NULL);
  assert(config->divisor >= 1 && config->divisor <= 16);
  assert(config->source != CLOCK_PLL);

  const struct PinEntry * const pinEntry = pinFind(clockOutputPins,
      config->pin, 0);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(config->pin);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  uint32_t value = CLKOUTCFG_EN | CLKOUTCFG_DIV(config->divisor - 1);

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      value |= CLKOUTCFG_IRC;
      break;

    case CLOCK_EXTERNAL:
      value |= CLKOUTCFG_MAIN;
      break;

    case CLOCK_RTC:
      value |= CLKOUTCFG_RTC;
      break;

    case CLOCK_USB_PLL:
      value |= CLKOUTCFG_USB;
      break;

    case CLOCK_MAIN:
      value |= CLKOUTCFG_CPU;
      break;

    default:
      break;
  }

  LPC_SC->CLKOUTCFG = value;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t clockOutputFrequency(const void *clockBase
    __attribute__((unused)))
{
  if (!(LPC_SC->CLKOUTCFG & CLKOUTCFG_ACT))
    return 0;

  const uint32_t divisor = CLKOUTCFG_DIV_VALUE(LPC_SC->CLKOUTCFG) + 1;
  uint32_t frequency = 0;

  switch (CLKOUTCFG_SEL_VALUE(LPC_SC->CLKOUTCFG))
  {
    case CLKOUTCFG_CPU:
      frequency = mainClockFrequency(NULL);
      break;

    case CLKOUTCFG_MAIN:
      frequency = extOscFrequency(NULL);
      break;

    case CLKOUTCFG_IRC:
      frequency = INT_OSC_FREQUENCY;
      break;

    case CLKOUTCFG_USB:
      frequency = usbClockFrequency(NULL);
      break;

    case CLKOUTCFG_RTC:
      frequency = RTC_OSC_FREQUENCY;
      break;

    default:
      break;
  }

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static bool clockOutputReady(const void *clockBase __attribute__((unused)))
{
  return (LPC_SC->CLKOUTCFG & CLKOUTCFG_ACT) != 0;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  flashLatencyReset();

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

    LPC_SC->CCLKCFG = 0;
    LPC_SC->CLKSRCSEL = source;
  }
  else
  {
    LPC_SC->CCLKCFG = pllDivisor - 1;

    /* Connect PLL */
    LPC_SC->PLL0CON |= PLL0CON_CONNECT;
    LPC_SC->PLL0FEED = PLLFEED_FIRST;
    LPC_SC->PLL0FEED = PLLFEED_SECOND;

    /* Wait for PLL enabled and connected */
    const uint32_t mask = PLL0STAT_ENABLED | PLL0STAT_CONNECTED;
    while ((LPC_SC->PLL0STAT & mask) != mask);
  }

  const uint32_t frequency = mainClockFrequency(NULL);

  flashLatencyUpdate(frequency);
  ticksPerSecond = TICK_RATE(frequency);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  const uint32_t source = LPC_SC->CLKSRCSEL;

  if (source == CLKSRCSEL_IRC)
  {
    return INT_OSC_FREQUENCY;
  }
  else
  {
    if (LPC_SC->PLL0STAT & PLL0STAT_CONNECTED)
    {
      /* Clock source is either main oscillator or RTC oscillator */
      return pllFrequency / pllDivisor;
    }
    else
    {
      if (source == CLKSRCSEL_MAIN)
        return extFrequency;
      else
        return RTC_OSC_FREQUENCY;
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result usbClockEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_PLL || config->source == CLOCK_USB_PLL);

  switch (config->source)
  {
    case CLOCK_PLL:
    {
      if (pllFrequency % USB_FREQUENCY != 0)
        return E_VALUE;

      const unsigned int divisor = pllFrequency / USB_FREQUENCY - 1;

      if (divisor != 5 && divisor != 7 && divisor != 9)
        return E_VALUE;

      LPC_SC->USBCLKCFG = USBCLKCFG_USBSEL(divisor);
      break;
    }

    case CLOCK_USB_PLL:
    {
      /* Connect PLL */
      LPC_SC->PLL1CON |= PLL1CON_CONNECT;
      LPC_SC->PLL1FEED = PLLFEED_FIRST;
      LPC_SC->PLL1FEED = PLLFEED_SECOND;

      /* Wait for PLL enabled and connected */
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
static uint32_t usbClockFrequency(const void *clockBase)
{
  return usbClockReady(clockBase) ? USB_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool usbClockReady(const void *clockBase __attribute__((unused)))
{
  const unsigned int actualDivisor = USBCLKCFG_USBSEL_VALUE(LPC_SC->USBCLKCFG);
  const unsigned int divisor = pllFrequency / USB_FREQUENCY - 1;

  return (actualDivisor && actualDivisor == divisor)
      || (LPC_SC->PLL1STAT & PLL1STAT_LOCK);
}
