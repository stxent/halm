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
#define EXT_OSC_FREQUENCY     24000000
#define TICK_RATE(frequency)  ((frequency) / 1000)
/*----------------------------------------------------------------------------*/
struct PfdClockClass
{
  struct ClockClass base;
  uint8_t pfd;
};
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *);
// static bool clockReadyStub(const void *);

static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

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
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &(const struct ClockClass){
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
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
/*----------------------------------------------------------------------------*/
uint32_t ticksPerSecond = TICK_RATE(EXT_OSC_FREQUENCY);
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *clockBase __attribute__((unused)))
{
}
/*----------------------------------------------------------------------------*/
// static bool clockReadyStub(const void *clockBase __attribute__((unused)))
// {
//   // TODO
//   return true;
// }
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *clockBase __attribute__((unused)))
{
  IMX_CCM_ANALOG->MISC0 |= MISC0_XTAL_24M_PWD;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *clockBase __attribute__((unused)),
    const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;

  if (config != NULL && config->current > OSC_CURRENT_MINUS_37P5)
    return E_VALUE;

  uint32_t misc0 = IMX_CCM_ANALOG->MISC0;

  misc0 &= ~(MISC0_OSC_I_MASK | MISC0_XTAL_24M_PWD);
  misc0 |= MISC0_OSC_XTALOK_EN;

  if (config != NULL)
    misc0 |= MISC0_OSC_I(config->current);

  IMX_CCM_ANALOG->MISC0 = misc0;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *clockBase __attribute__((unused)))
{
  return extOscReady(NULL) ? EXT_OSC_FREQUENCY : 0;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *clockBase __attribute__((unused)))
{
  const uint32_t misc0 = IMX_CCM_ANALOG->MISC0;

  if (misc0 & MISC0_XTAL_24M_PWD)
    return false;

  return misc0 & MISC0_OSC_XTALOK;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > 8)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  const uint32_t frequency = periphClockFrequency(NULL);
  uint32_t cbcdr = IMX_CCM->CBCDR;

  cbcdr = (cbcdr & ~CBCDR_AHB_PODF_MASK) | CBCDR_AHB_PODF(divisor - 1);
  ticksPerSecond = TICK_RATE(frequency / divisor);

  IMX_CCM->CBCDR = cbcdr;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  const uint32_t divisor = CBCDR_AHB_PODF_VALUE(IMX_CCM->CBCDR) + 1;
  return periphClockFrequency(NULL) / divisor;
}
/*----------------------------------------------------------------------------*/
static bool mainClockReady(const void *clockBase __attribute__((unused)))
{
  return !(IMX_CCM->CDHIPR & CDHIPR_AHB_PODF_BUSY);
}
/*----------------------------------------------------------------------------*/
static enum Result periphClockEnable(const void *clockBase
    __attribute__((unused)), const void *configBase)
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
static uint32_t periphClockFrequency(const void *clockBase
    __attribute__((unused)))
{
  const uint32_t cbcdr = IMX_CCM->CBCDR;

  if (cbcdr & CBCDR_PERIPH_CLK_SEL)
  {
    const uint32_t divisor = CBCDR_PERIPH_CLK2_PODF_VALUE(cbcdr) + 1;
    const uint32_t source = CBCMR_PERIPH_CLK2_SEL_VALUE(IMX_CCM->CBCMR);

    if (source == PERIPH_CLK2_SEL_OSC)
    {
      return EXT_OSC_FREQUENCY / divisor;
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
static bool periphClockReady(const void *clockBase __attribute__((unused)))
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
static void pll1Disable(const void *clockBase __attribute__((unused)))
{
  IMX_CCM_ANALOG->PLL_ARM_SET = PLL_POWERDOWN | PLL_BYPASS;
}
/*----------------------------------------------------------------------------*/
static enum Result pll1Enable(const void *clockBase __attribute__((unused)),
    const void *configBase)
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
static uint32_t pll1Frequency(const void *clockBase __attribute__((unused)))
{
  const uint32_t value = IMX_CCM_ANALOG->PLL_ARM;
  const uint32_t divisor = PLL_ARM_DIV_SELECT_VALUE(value);

  if ((value & (PLL_POWERDOWN | PLL_LOCK)) == PLL_LOCK)
    return EXT_OSC_FREQUENCY * divisor / 2;
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static bool pll1Ready(const void *clockBase __attribute__((unused)))
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

    return EXT_OSC_FREQUENCY * pllDivisor / pfdDivisor * 18;
  }
  else
    return EXT_OSC_FREQUENCY * pllDivisor;
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
    uint32_t value = PLL_ENABLE | PLL_USB_POWER
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

    return EXT_OSC_FREQUENCY * pllDivisor / pfdDivisor * 18;
  }
  else
    return EXT_OSC_FREQUENCY * pllDivisor;
}
/*----------------------------------------------------------------------------*/
static bool pll3Ready(const void *clockBase)
{
  static const uint32_t READY_MASK = PLL_USB_POWER | PLL_LOCK;
  const struct PfdClockClass * const clock = clockBase;

  if ((IMX_CCM_ANALOG->PLL_USB1 & READY_MASK) != READY_MASK)
    return false;

  if (clock->pfd < 4)
    return !(IMX_CCM_ANALOG->PFD_480 & PLL_PFD_PFDn_CLKGATE(clock->pfd));

  return true;
}
