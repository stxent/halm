/*
 * clocking.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/clocking.h>
#include <halm/platform/imxrt/imxrt106x/clocking_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 24000000
#define EXT_OSC_FREQUENCY 24000000
#define OSC_FREQUENCY     (EXT_OSC_FREQUENCY)

#define TICK_RATE(frequency) \
    (((frequency) * 4398046ULL + (1ULL << 31)) >> 32)
/*----------------------------------------------------------------------------*/
struct PfdClockClass
{
  struct ClockClass base;
  uint8_t pfd;
};
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *);
static bool clockReadyStub(const void *);

static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void intOscDisable(const void *);
static enum Result intOscEnable(const void *, const void *);
static uint32_t intOscFrequency(const void *);
static bool intOscReady(const void *);

static enum Result ipgClockEnable(const void *, const void *);
static uint32_t ipgClockFrequency(const void *);

static enum Result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);
static bool mainClockReady(const void *);

static enum Result periphClockEnable(const void *, const void *);
static uint32_t periphClockFrequency(const void *);
static bool periphClockReady(const void *);

static void pll1Disable(const void *);
static enum Result pll1Enable(const void *, const void *);
static uint32_t pll1Frequency(const void *);
static bool pll1Ready(const void *);

static void pll2Disable(const void *);
static enum Result pll2Enable(const void *, const void *);
static uint32_t pll2Frequency(const void *);
static bool pll2Ready(const void *);

static void pll3Disable(const void *);
static enum Result pll3Enable(const void *, const void *);
static uint32_t pll3Frequency(const void *);
static bool pll3Ready(const void *);

static void pll7Disable(const void *);
static enum Result pll7Enable(const void *, const void *);
static uint32_t pll7Frequency(const void *);
static bool pll7Ready(const void *);

static enum Result flexSpi1ClockEnable(const void *, const void *);
static uint32_t flexSpi1ClockFrequency(const void *);

static enum Result flexSpi2ClockEnable(const void *, const void *);
static uint32_t flexSpi2ClockFrequency(const void *);

static enum Result timerClockEnable(const void *, const void *);
static uint32_t timerClockFrequency(const void *);

static enum Result uartClockEnable(const void *, const void *);
static uint32_t uartClockFrequency(const void *);
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

const struct ClockClass * const IpgClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = ipgClockEnable,
    .frequency = ipgClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const MainClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = mainClockReady
};

const struct ClockClass * const PeriphClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = periphClockEnable,
    .frequency = periphClockFrequency,
    .ready = periphClockReady
};

const struct ClockClass * const ArmPll = &(const struct ClockClass){
    .disable = pll1Disable,
    .enable = pll1Enable,
    .frequency = pll1Frequency,
    .ready = pll1Ready
};

const struct ClockClass * const SystemPll =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll2Disable,
        .enable = pll2Enable,
        .frequency = pll2Frequency,
        .ready = pll2Ready
    },
    .pfd = UINT8_MAX
};

const struct ClockClass * const SystemPllPfd0 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll2Disable,
        .enable = pll2Enable,
        .frequency = pll2Frequency,
        .ready = pll2Ready
    },
    .pfd = 0
};

const struct ClockClass * const SystemPllPfd1 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll2Disable,
        .enable = pll2Enable,
        .frequency = pll2Frequency,
        .ready = pll2Ready
    },
    .pfd = 1
};

const struct ClockClass * const SystemPllPfd2 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll2Disable,
        .enable = pll2Enable,
        .frequency = pll2Frequency,
        .ready = pll2Ready,
    },
    .pfd = 2
};

const struct ClockClass * const SystemPllPfd3 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll2Disable,
        .enable = pll2Enable,
        .frequency = pll2Frequency,
        .ready = pll2Ready,
    },
    .pfd = 3
};

const struct ClockClass * const Usb1Pll =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll3Disable,
        .enable = pll3Enable,
        .frequency = pll3Frequency,
        .ready = pll3Ready
    },
    .pfd = UINT8_MAX
};

const struct ClockClass * const Usb1PllPfd0 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll3Disable,
        .enable = pll3Enable,
        .frequency = pll3Frequency,
        .ready = pll3Ready
    },
    .pfd = 0
};

const struct ClockClass * const Usb1PllPfd1 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll3Disable,
        .enable = pll3Enable,
        .frequency = pll3Frequency,
        .ready = pll3Ready
    },
    .pfd = 1
};

const struct ClockClass * const Usb1PllPfd2 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll3Disable,
        .enable = pll3Enable,
        .frequency = pll3Frequency,
        .ready = pll3Ready,
    },
    .pfd = 2
};

const struct ClockClass * const Usb1PllPfd3 =
    (const struct ClockClass *)&(const struct PfdClockClass){
    .base = {
        .disable = pll3Disable,
        .enable = pll3Enable,
        .frequency = pll3Frequency,
        .ready = pll3Ready,
    },
    .pfd = 3
};

const struct ClockClass * const Usb2Pll = &(const struct ClockClass){
    .disable = pll7Disable,
    .enable = pll7Enable,
    .frequency = pll7Frequency,
    .ready = pll7Ready
};

const struct ClockClass * const FlexSpi1Clock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = flexSpi1ClockEnable,
    .frequency = flexSpi1ClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const FlexSpi2Clock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = flexSpi2ClockEnable,
    .frequency = flexSpi2ClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const TimerClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = timerClockEnable,
    .frequency = timerClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const UartClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = uartClockEnable,
    .frequency = uartClockFrequency,
    .ready = clockReadyStub
};
/*----------------------------------------------------------------------------*/
uint32_t ticksPerSecond = TICK_RATE(OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *)
{
}
/*----------------------------------------------------------------------------*/
static bool clockReadyStub(const void *)
{
  return true;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *)
{
  IMX_XTALOSC24M->MISC0_SET = MISC0_XTAL_24M_PWD;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *, const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;

  if (config != NULL && (config->current > OSC_CURRENT_MINUS_37P5
      || config->delay > OSC_DELAY_2MS))
  {
    return E_VALUE;
  }

  uint32_t misc = IMX_XTALOSC24M->MISC0 & ~MISC0_XTAL_24M_PWD;
  uint32_t pwr = IMX_XTALOSC24M->LOWPWR_CTRL;

  if (config != NULL)
  {
    misc = (misc & ~MISC0_OSC_I_MASK) | MISC0_OSC_I(config->current);
    pwr &= ~LOWPWR_CTRL_XTALOSC_PWRUP_DELAY_MASK;

    if (config->delay != OSC_DELAY_DEFAULT)
      pwr |= LOWPWR_CTRL_XTALOSC_PWRUP_DELAY(config->delay - 1);
    else
      pwr |= LOWPWR_CTRL_XTALOSC_PWRUP_DELAY(XTALOSC_PWRUP_DELAY_2MS);
  }

  IMX_XTALOSC24M->LOWPWR_CTRL = pwr;
  IMX_XTALOSC24M->MISC0 = misc;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *)
{
  return extOscReady(NULL) ? OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *)
{
  if (IMX_XTALOSC24M->MISC0 & MISC0_XTAL_24M_PWD)
    return false;

  return (IMX_XTALOSC24M->LOWPWR_CTRL & LOWPWR_CTRL_XTALOSC_PWRUP_STAT) != 0;
}
/*----------------------------------------------------------------------------*/
static void intOscDisable(const void *)
{
  IMX_XTALOSC24M->LOWPWR_CTRL_CLR = LOWPWR_CTRL_OSC_SEL;
  IMX_XTALOSC24M->LOWPWR_CTRL_CLR = LOWPWR_CTRL_RC_OSC_EN;
}
/*----------------------------------------------------------------------------*/
static enum Result intOscEnable(const void *, const void *)
{
  IMX_XTALOSC24M->OSC_CONFIG2_SET = OSC_CONFIG2_ENABLE_1M;
  IMX_XTALOSC24M->OSC_CONFIG2_CLR = OSC_CONFIG2_MUX_1M;

  IMX_XTALOSC24M->LOWPWR_CTRL_SET = LOWPWR_CTRL_RC_OSC_EN;
  IMX_XTALOSC24M->LOWPWR_CTRL_SET = LOWPWR_CTRL_OSC_SEL;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t intOscFrequency(const void *)
{
  return (IMX_XTALOSC24M->LOWPWR_CTRL & LOWPWR_CTRL_RC_OSC_EN) ?
      INT_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool intOscReady(const void *)
{
  return (IMX_XTALOSC24M->LOWPWR_CTRL & LOWPWR_CTRL_RC_OSC_EN) != 0;
}
/*----------------------------------------------------------------------------*/
static enum Result ipgClockEnable(const void *, const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 4)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t cbcdr = IMX_CCM->CBCDR;

  cbcdr = (cbcdr & ~CBCDR_IPG_PODF_MASK) | CBCDR_IPG_PODF(divisor - 1);
  IMX_CCM->CBCDR = cbcdr;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t ipgClockFrequency(const void *)
{
  const uint32_t divisor = CBCDR_IPG_PODF_VALUE(IMX_CCM->CBCDR) + 1;
  return mainClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *, const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 8)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  const uint32_t frequency = periphClockFrequency(NULL);
  uint32_t cbcdr = IMX_CCM->CBCDR;

  cbcdr = (cbcdr & ~CBCDR_AHB_PODF_MASK) | CBCDR_AHB_PODF(divisor - 1);
  IMX_CCM->CBCDR = cbcdr;

  ticksPerSecond = TICK_RATE(frequency / divisor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *)
{
  const uint32_t divisor = CBCDR_AHB_PODF_VALUE(IMX_CCM->CBCDR) + 1;
  return periphClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static bool mainClockReady(const void *)
{
  return !(IMX_CCM->CDHIPR & CDHIPR_AHB_PODF_BUSY);
}
/*----------------------------------------------------------------------------*/
static enum Result periphClockEnable(const void *, const void *configBase)
{
  const struct ExtendedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->source == CLOCK_OSC || config->source == CLOCK_USB1_PLL)
  {
    if (config->divisor > 8)
      return E_VALUE;

    uint32_t cbcdr = IMX_CCM->CBCDR & ~CBCDR_PERIPH_CLK2_PODF_MASK;
    uint32_t cbcmr = IMX_CCM->CBCMR & ~CBCMR_PERIPH_CLK2_SEL_MASK;

    cbcdr |= CBCDR_PERIPH_CLK_SEL;
    if (config->divisor > 1)
      cbcdr |= CBCDR_PERIPH_CLK2_PODF(config->divisor - 1);

    if (config->source == CLOCK_OSC)
      cbcmr |= CBCMR_PERIPH_CLK2_SEL(PERIPH_CLK2_SEL_OSC);
    else
      cbcmr |= CBCMR_PERIPH_CLK2_SEL(PERIPH_CLK2_SEL_PLL3_SW);

    IMX_CCM->CBCMR = cbcmr;
    IMX_CCM->CBCDR = cbcdr;
  }
  else
  {
    const uint32_t cbcdr = IMX_CCM->CBCDR & ~CBCDR_PERIPH_CLK_SEL;
    uint32_t cacrr = IMX_CCM->CACRR;
    uint32_t cbcmr = IMX_CCM->CBCMR & ~CBCMR_PRE_PERIPH_CLK_SEL_MASK;

    if (config->source == CLOCK_ARM_PLL)
    {
      if (config->divisor > 8)
        return E_VALUE;

      cacrr &= ~CACRR_ARM_PODF_MASK;
      if (config->divisor > 1)
        cacrr |= CACRR_ARM_PODF(config->divisor - 1);

      cbcmr |= CBCMR_PRE_PERIPH_CLK_SEL(PRE_PERIPH_CLK_SEL_PLL1_DIV);
    }
    else if (config->source == CLOCK_SYSTEM_PLL)
    {
      if (config->divisor > 1)
        return E_VALUE;

      cbcmr |= CBCMR_PRE_PERIPH_CLK_SEL(PRE_PERIPH_CLK_SEL_PLL2);
    }
    else if (config->source == CLOCK_SYSTEM_PLL_PFD0)
    {
      if (config->divisor > 1)
        return E_VALUE;

      cbcmr |= CBCMR_PRE_PERIPH_CLK_SEL(PRE_PERIPH_CLK_SEL_PLL2_PFD0);
    }
    else if (config->source == CLOCK_SYSTEM_PLL_PFD2)
    {
      if (config->divisor > 1)
        return E_VALUE;

      cbcmr |= CBCMR_PRE_PERIPH_CLK_SEL(PRE_PERIPH_CLK_SEL_PLL2_PFD2);
    }
    else
      return E_VALUE;

    IMX_CCM->CACRR = cacrr;
    IMX_CCM->CBCMR = cbcmr;
    IMX_CCM->CBCDR = cbcdr;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t periphClockFrequency(const void *)
{
  const uint32_t cbcdr = IMX_CCM->CBCDR;

  if (cbcdr & CBCDR_PERIPH_CLK_SEL)
  {
    const uint32_t divisor = CBCDR_PERIPH_CLK2_PODF_VALUE(cbcdr) + 1;
    const uint32_t source = CBCMR_PERIPH_CLK2_SEL_VALUE(IMX_CCM->CBCMR);

    if (source == PERIPH_CLK2_SEL_OSC)
    {
      return OSC_FREQUENCY / divisor;
    }
    else
    {
      /* PLL3 main clock */
      return pll3Frequency(Usb1Pll);
    }
  }
  else
  {
    switch (CBCMR_PRE_PERIPH_CLK_SEL_VALUE(IMX_CCM->CBCMR))
    {
      case PRE_PERIPH_CLK_SEL_PLL2:
        return pll2Frequency(SystemPll);

      case PRE_PERIPH_CLK_SEL_PLL2_PFD2:
        return pll2Frequency(SystemPllPfd2);

      case PRE_PERIPH_CLK_SEL_PLL2_PFD0:
        return pll2Frequency(SystemPllPfd0);

      case PRE_PERIPH_CLK_SEL_PLL1_DIV:
      {
        const uint32_t frequency = pll1Frequency(NULL);
        const uint32_t divisor = CACRR_ARM_PODF_VALUE(IMX_CCM->CACRR) + 1;

        return frequency / divisor;
      }
    }
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
static bool periphClockReady(const void *)
{
  uint32_t mask = CDHIPR_PERIPH_CLK_SEL_BUSY;

  if (!(IMX_CCM->CBCDR & CBCDR_PERIPH_CLK_SEL))
  {
    const uint32_t source = CBCMR_PRE_PERIPH_CLK_SEL_VALUE(IMX_CCM->CBCMR);

    if (source == PRE_PERIPH_CLK_SEL_PLL1_DIV)
      mask |= CDHIPR_ARM_PODF_BUSY;
  }
  else
    mask |= CDHIPR_PERIPH2_CLK_SEL_BUSY;

  return !(IMX_CCM->CDHIPR & mask);
}
/*----------------------------------------------------------------------------*/
static void pll1Disable(const void *)
{
  IMX_CCM_ANALOG->PLL_ARM_SET = PLL_POWERDOWN | PLL_BYPASS;
}
/*----------------------------------------------------------------------------*/
static enum Result pll1Enable(const void *, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor < 54 || config->divisor > 108)
    return E_VALUE;

  IMX_CCM_ANALOG->PLL_ARM_SET = PLL_POWERDOWN | PLL_BYPASS;
  IMX_CCM_ANALOG->PLL_ARM = PLL_ARM_DIV_SELECT(config->divisor) | PLL_ENABLE
      | PLL_BYPASS_CLK_SRC(BYPASS_CLK_SRC_REF_CLK_24M);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll1Frequency(const void *)
{
  const uint32_t value = IMX_CCM_ANALOG->PLL_ARM;
  const uint32_t divisor = PLL_ARM_DIV_SELECT_VALUE(value);

  if ((value & (PLL_POWERDOWN | PLL_LOCK)) == PLL_LOCK)
    return OSC_FREQUENCY * divisor / 2;
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static bool pll1Ready(const void *)
{
  return (IMX_CCM_ANALOG->PLL_ARM & (PLL_POWERDOWN | PLL_LOCK)) == PLL_LOCK;
}
/*----------------------------------------------------------------------------*/
static void pll2Disable(const void *clockBase)
{
  const struct PfdClockClass * const clock = clockBase;

  if (clock->pfd < 4)
    IMX_CCM_ANALOG->PFD_528_SET = PLL_PFD_PFDn_CLKGATE(clock->pfd);
  else
    IMX_CCM_ANALOG->PLL_SYS_SET = PLL_POWERDOWN | PLL_BYPASS;
}
/*----------------------------------------------------------------------------*/
static enum Result pll2Enable(const void *clockBase, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);

  const struct PfdClockClass * const clock = clockBase;

  if (clock->pfd < 4)
  {
    if (config->divisor < 12 || config->divisor > 35)
      return E_VALUE;

    uint32_t value = IMX_CCM_ANALOG->PFD_528;

    value &= ~PLL_PFD_PFDn_CLKGATE(clock->pfd);
    value &= ~PLL_PFD_PFDn_FRAC_MASK(clock->pfd);
    value |= PLL_PFD_PFDn_FRAC(config->divisor, clock->pfd);

    IMX_CCM_ANALOG->PFD_528 = value;
  }
  else
  {
    uint32_t value = PLL_ENABLE
        | PLL_BYPASS_CLK_SRC(BYPASS_CLK_SRC_REF_CLK_24M);

    if (config->divisor == 22)
      value |= PLL_SYS_DIV_SELECT;
    else if (config->divisor != 20)
      return E_VALUE;

    IMX_CCM_ANALOG->PFD_528_SET =
        PLL_PFD_PFDn_CLKGATE(0)
        | PLL_PFD_PFDn_CLKGATE(1)
        | PLL_PFD_PFDn_CLKGATE(2)
        | PLL_PFD_PFDn_CLKGATE(3);

    IMX_CCM_ANALOG->PLL_SYS_SET = PLL_POWERDOWN | PLL_BYPASS;
    IMX_CCM_ANALOG->PLL_SYS = value;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll2Frequency(const void *clockBase)
{
  const struct PfdClockClass * const clock = clockBase;
  const uint32_t pllControl = IMX_CCM_ANALOG->PLL_SYS;
  const uint32_t pllDivisor = (pllControl & PLL_SYS_DIV_SELECT) ? 22 : 20;

  if ((pllControl & (PLL_POWERDOWN | PLL_LOCK)) != PLL_LOCK)
    return 0;

  if (clock->pfd < 4)
  {
    const uint32_t pfdControl = IMX_CCM_ANALOG->PFD_528;
    const uint32_t pfdDivisor = PLL_PFD_PFDn_FRAC_VALUE(pfdControl, clock->pfd);

    if (pfdControl & PLL_PFD_PFDn_CLKGATE(clock->pfd))
      return 0;

    /* Source: OSC_FREQUENCY * PLL_DIV * 18 / PFD_FRAC */
    return (((OSC_FREQUENCY >> 2) * 18 * pllDivisor) / pfdDivisor) << 2;
  }
  else
    return OSC_FREQUENCY * pllDivisor;
}
/*----------------------------------------------------------------------------*/
static bool pll2Ready(const void *clockBase)
{
  const struct PfdClockClass * const clock = clockBase;

  if ((IMX_CCM_ANALOG->PLL_SYS & (PLL_POWERDOWN | PLL_LOCK)) != PLL_LOCK)
    return false;

  if (clock->pfd < 4)
    return !(IMX_CCM_ANALOG->PFD_528 & PLL_PFD_PFDn_CLKGATE(clock->pfd));

  return true;
}
/*----------------------------------------------------------------------------*/
static void pll3Disable(const void *clockBase)
{
  const struct PfdClockClass * const clock = clockBase;

  if (clock->pfd < 4)
    IMX_CCM_ANALOG->PFD_480_SET = PLL_PFD_PFDn_CLKGATE(clock->pfd);
  else
    IMX_CCM_ANALOG->PLL_USB1_SET = PLL_POWERDOWN | PLL_BYPASS;
}
/*----------------------------------------------------------------------------*/
static enum Result pll3Enable(const void *clockBase, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);

  const struct PfdClockClass * const clock = clockBase;

  if (clock->pfd < 4)
  {
    if (config->divisor < 12 || config->divisor > 35)
      return E_VALUE;

    uint32_t value = IMX_CCM_ANALOG->PFD_480;

    value &= ~PLL_PFD_PFDn_CLKGATE(clock->pfd);
    value &= ~PLL_PFD_PFDn_FRAC_MASK(clock->pfd);
    value |= PLL_PFD_PFDn_FRAC(config->divisor, clock->pfd);

    IMX_CCM_ANALOG->PFD_480 = value;
  }
  else
  {
    uint32_t value = PLL_USB_EN_USB_CLKS | PLL_USB_POWER | PLL_ENABLE
        | PLL_BYPASS_CLK_SRC(BYPASS_CLK_SRC_REF_CLK_24M);

    if (config->divisor == 22)
      value |= PLL_USB_DIV_SELECT;
    else if (config->divisor != 20)
      return E_VALUE;

    IMX_CCM_ANALOG->PFD_480_SET =
        PLL_PFD_PFDn_CLKGATE(0)
        | PLL_PFD_PFDn_CLKGATE(1)
        | PLL_PFD_PFDn_CLKGATE(2)
        | PLL_PFD_PFDn_CLKGATE(3);

    IMX_CCM->CCSR &= ~CCSR_PLL3_SW_CLK_SEL;
    IMX_CCM_ANALOG->PLL_USB1_SET = PLL_BYPASS;
    IMX_CCM_ANALOG->PLL_USB1_CLR = PLL_USB_POWER;
    IMX_CCM_ANALOG->PLL_USB1 = value;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll3Frequency(const void *clockBase)
{
  const struct PfdClockClass * const clock = clockBase;
  const uint32_t pllControl = IMX_CCM_ANALOG->PLL_USB1;
  const uint32_t pllDivisor = (pllControl & PLL_USB_DIV_SELECT) ? 22 : 20;

  if ((pllControl & (PLL_USB_POWER | PLL_LOCK)) != (PLL_USB_POWER | PLL_LOCK))
    return 0;

  if (clock->pfd < 4)
  {
    const uint32_t pfdControl = IMX_CCM_ANALOG->PFD_480;
    const uint32_t pfdDivisor = PLL_PFD_PFDn_FRAC_VALUE(pfdControl, clock->pfd);

    if (pfdControl & PLL_PFD_PFDn_CLKGATE(clock->pfd))
      return 0;

    /* Source: OSC_FREQUENCY * PLL_DIV * 18 / PFD_FRAC */
    return (((OSC_FREQUENCY >> 2) * 18 * pllDivisor) / pfdDivisor) << 2;
  }
  else
    return OSC_FREQUENCY * pllDivisor;
}
/*----------------------------------------------------------------------------*/
static bool pll3Ready(const void *clockBase)
{
  static const uint32_t pllReadyMask = PLL_USB_POWER | PLL_LOCK;
  const struct PfdClockClass * const clock = clockBase;

  if ((IMX_CCM_ANALOG->PLL_USB1 & pllReadyMask) != pllReadyMask)
    return false;

  if (clock->pfd < 4)
    return !(IMX_CCM_ANALOG->PFD_480 & PLL_PFD_PFDn_CLKGATE(clock->pfd));

  return true;
}
/*----------------------------------------------------------------------------*/
static void pll7Disable(const void *)
{
  IMX_CCM_ANALOG->PLL_USB2_SET = PLL_POWERDOWN | PLL_BYPASS;
}
/*----------------------------------------------------------------------------*/
static enum Result pll7Enable(const void *, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);

  uint32_t value = PLL_USB_EN_USB_CLKS | PLL_USB_POWER | PLL_ENABLE
      | PLL_BYPASS_CLK_SRC(BYPASS_CLK_SRC_REF_CLK_24M);

  if (config->divisor == 22)
    value |= PLL_USB_DIV_SELECT;
  else if (config->divisor != 20)
    return E_VALUE;

  IMX_CCM_ANALOG->PLL_USB2_SET = PLL_BYPASS;
  IMX_CCM_ANALOG->PLL_USB2_CLR = PLL_USB_POWER;
  IMX_CCM_ANALOG->PLL_USB2 = value;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t pll7Frequency(const void *)
{
  const uint32_t pllControl = IMX_CCM_ANALOG->PLL_USB2;
  const uint32_t pllDivisor = (pllControl & PLL_USB_DIV_SELECT) ? 22 : 20;

  if ((pllControl & (PLL_USB_POWER | PLL_LOCK)) != (PLL_USB_POWER | PLL_LOCK))
    return 0;

  return OSC_FREQUENCY * pllDivisor;
}
/*----------------------------------------------------------------------------*/
static bool pll7Ready(const void *)
{
  static const uint32_t pllReadyMask = PLL_USB_POWER | PLL_LOCK;
  return (IMX_CCM_ANALOG->PLL_USB2 & pllReadyMask) == pllReadyMask;
}
/*----------------------------------------------------------------------------*/
static enum Result flexSpi1ClockEnable(const void *, const void *configBase)
{
  const struct ExtendedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 8)
    return E_VALUE;

  uint32_t cscmr1 = IMX_CCM->CSCMR1
      & ~(CSCMR1_FLEXSPI_CLK_SEL_MASK | CSCMR1_FLEXSPI_PODF_MASK);

  switch (config->source)
  {
    case CLOCK_SYSTEM_PLL_PFD2:
      cscmr1 |= CSCMR1_FLEXSPI_CLK_SEL(FLEXSPI_CLK_SEL_PLL2_PFD2);
      break;

    case CLOCK_USB1_PLL:
      cscmr1 |= CSCMR1_FLEXSPI_CLK_SEL(FLEXSPI_CLK_SEL_PLL3_SW);
      break;

    case CLOCK_USB1_PLL_PFD0:
      cscmr1 |= CSCMR1_FLEXSPI_CLK_SEL(FLEXSPI_CLK_SEL_PLL3_PFD0);
      break;

    case CLOCK_SEMC:
      cscmr1 |= CSCMR1_FLEXSPI_CLK_SEL(FLEXSPI_CLK_SEL_SEMC_ROOT_PRE);
      break;

    default:
      return E_VALUE;
  }

  if (config->divisor > 1)
    cscmr1 |= CSCMR1_FLEXSPI_PODF(config->divisor - 1);

  IMX_CCM->CSCMR1 = cscmr1;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t flexSpi1ClockFrequency(const void *)
{
  const uint32_t cscmr1 = IMX_CCM->CSCMR1;
  const uint32_t divisor = CSCMR1_FLEXSPI_PODF_VALUE(cscmr1) + 1;
  uint32_t frequency = 0;

  switch (CSCMR1_FLEXSPI_CLK_SEL_VALUE(cscmr1))
  {
    case FLEXSPI_CLK_SEL_SEMC_ROOT_PRE:
      // TODO
      break;

    case FLEXSPI_CLK_SEL_PLL3_SW:
      frequency = pll3Frequency(Usb1Pll);
      break;

    case FLEXSPI_CLK_SEL_PLL2_PFD2:
      frequency = pll2Frequency(SystemPllPfd2);
      break;

    case FLEXSPI_CLK_SEL_PLL3_PFD0:
      frequency = pll3Frequency(Usb1PllPfd0);
      break;

    default:
      break;
  }

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result flexSpi2ClockEnable(const void *, const void *configBase)
{
  const struct ExtendedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 8)
    return E_VALUE;

  uint32_t cbcmr = IMX_CCM->CBCMR
      & ~(CBCMR_FLEXSPI2_CLK_SEL_MASK | CBCMR_FLEXSPI2_PODF_MASK);

  switch (config->source)
  {
    case CLOCK_SYSTEM_PLL:
      cbcmr |= CBCMR_FLEXSPI2_CLK_SEL(FLEXSPI2_CLK_SEL_PLL2);
      break;

    case CLOCK_SYSTEM_PLL_PFD2:
      cbcmr |= CBCMR_FLEXSPI2_CLK_SEL(FLEXSPI2_CLK_SEL_PLL2_PFD2);
      break;

    case CLOCK_USB1_PLL_PFD0:
      cbcmr |= CBCMR_FLEXSPI2_CLK_SEL(FLEXSPI2_CLK_SEL_PLL3_PFD0);
      break;

    case CLOCK_USB1_PLL_PFD1:
      cbcmr |= CBCMR_FLEXSPI2_CLK_SEL(FLEXSPI2_CLK_SEL_PLL3_PFD1);
      break;

    default:
      return E_VALUE;
  }

  if (config->divisor > 1)
    cbcmr |= CBCMR_FLEXSPI2_PODF(config->divisor - 1);

  IMX_CCM->CBCMR = cbcmr;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t flexSpi2ClockFrequency(const void *)
{
  const uint32_t cbcmr = IMX_CCM->CBCMR;
  const uint32_t divisor = CBCMR_FLEXSPI2_PODF_VALUE(cbcmr) + 1;
  uint32_t frequency = 0;

  switch (CBCMR_FLEXSPI2_CLK_SEL_VALUE(cbcmr))
  {
    case FLEXSPI2_CLK_SEL_PLL2_PFD2:
      frequency = pll2Frequency(SystemPllPfd2);
      break;

    case FLEXSPI2_CLK_SEL_PLL3_PFD0:
      frequency = pll3Frequency(Usb1PllPfd0);
      break;

    case FLEXSPI2_CLK_SEL_PLL3_PFD1:
      frequency = pll3Frequency(Usb1PllPfd1);
      break;

    case FLEXSPI2_CLK_SEL_PLL2:
      frequency = pll2Frequency(SystemPll);
      break;

    default:
      break;
  }

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result timerClockEnable(const void *, const void *configBase)
{
  const struct ExtendedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 64)
    return E_VALUE;

  uint32_t cscmr1 = IMX_CCM->CSCMR1 & ~CSCMR1_PERCLK_PODF_MASK;

  if (config->source == CLOCK_OSC)
    cscmr1 |= CSCMR1_PERCLK_CLK_SEL;
  else if (config->source == CLOCK_IPG)
    cscmr1 &= ~CSCMR1_PERCLK_CLK_SEL;
  else
    return E_VALUE;

  if (config->divisor > 1)
    cscmr1 |= CSCMR1_PERCLK_PODF(config->divisor - 1);

  IMX_CCM->CSCMR1 = cscmr1;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t timerClockFrequency(const void *)
{
  const uint32_t cscmr1 = IMX_CCM->CSCMR1;
  const uint32_t divisor = CSCMR1_PERCLK_PODF_VALUE(cscmr1) + 1;
  uint32_t frequency;

  if (cscmr1 & CSCMR1_PERCLK_CLK_SEL)
    frequency = OSC_FREQUENCY;
  else
    frequency = ipgClockFrequency(NULL);

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result uartClockEnable(const void *, const void *configBase)
{
  const struct ExtendedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 64)
    return E_VALUE;

  uint32_t cscdr1 = IMX_CCM->CSCDR1 & ~CSCDR1_UART_CLK_PODF_MASK;

  if (config->source == CLOCK_OSC)
    cscdr1 |= CSCDR1_UART_CLK_SEL;
  else if (config->source == CLOCK_USB1_PLL)
    cscdr1 &= ~CSCDR1_UART_CLK_SEL;
  else
    return E_VALUE;

  if (config->divisor > 1)
    cscdr1 |= CSCDR1_UART_CLK_PODF(config->divisor - 1);

  IMX_CCM->CSCDR1 = cscdr1;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t uartClockFrequency(const void *)
{
  const uint32_t cscdr1 = IMX_CCM->CSCDR1;
  const uint32_t divisor = CSCDR1_UART_CLK_PODF_VALUE(cscdr1) + 1;

  if (!(cscdr1 & CSCDR1_UART_CLK_SEL))
  {
    const uint32_t frequency = pll3Frequency(Usb1Pll);
    return frequency / (6 * divisor);
  }
  else
    return OSC_FREQUENCY / divisor;
}
