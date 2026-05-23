/*
 * clocking.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/bouffalo/clocking.h>
#include <halm/platform/bouffalo/bl602/clocking_defs.h>
#include <halm/platform/bouffalo/bl602/l1c.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
#define INT_OSC_FREQUENCY 32000000

#define TICK_RATE(frequency) \
    (((frequency) * 4398046ULL + (1ULL << 31)) >> 32)
/*----------------------------------------------------------------------------*/
static void clockDisableStub(const void *);
static bool clockReadyStub(const void *);
static uint32_t rootClockFrequency(uint32_t);

static void extOscDisable(const void *);
static enum Result extOscEnable(const void *, const void *);
static uint32_t extOscFrequency(const void *);
static bool extOscReady(const void *);

static void flashClockDisable(const void *);
static enum Result flashClockEnable(const void *, const void *);
static uint32_t flashClockFrequency(const void *);
static bool flashClockReady(const void *);

static void i2cClockDisable(const void *);
static enum Result i2cClockEnable(const void *, const void *);
static uint32_t i2cClockFrequency(const void *);
static bool i2cClockReady(const void *);

static enum Result mainClockEnable(const void *, const void *);
static uint32_t mainClockFrequency(const void *);

static enum Result socClockEnable(const void *, const void *);
static uint32_t socClockFrequency(const void *);

static void spiClockDisable(const void *);
static enum Result spiClockEnable(const void *, const void *);
static uint32_t spiClockFrequency(const void *);
static bool spiClockReady(const void *);

static void sysPllDisable(const void *);
static enum Result sysPllEnable(const void *, const void *);
static uint32_t sysPllFrequency(const void *);
static bool sysPllReady(const void *);

static void uartClockDisable(const void *);
static enum Result uartClockEnable(const void *, const void *);
static uint32_t uartClockFrequency(const void *);
static bool uartClockReady(const void *);
/*----------------------------------------------------------------------------*/
const struct ClockClass * const ExternalOsc = &(const struct ClockClass){
    .disable = extOscDisable,
    .enable = extOscEnable,
    .frequency = extOscFrequency,
    .ready = extOscReady
};

const struct ClockClass * const FlashClock = &(const struct ClockClass){
    .disable = flashClockDisable,
    .enable = flashClockEnable,
    .frequency = flashClockFrequency,
    .ready = flashClockReady
};

const struct ClockClass * const I2CClock = &(const struct ClockClass){
    .disable = i2cClockDisable,
    .enable = i2cClockEnable,
    .frequency = i2cClockFrequency,
    .ready = i2cClockReady
};

const struct ClockClass * const MainClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = mainClockEnable,
    .frequency = mainClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const SocClock = &(const struct ClockClass){
    .disable = clockDisableStub,
    .enable = socClockEnable,
    .frequency = socClockFrequency,
    .ready = clockReadyStub
};

const struct ClockClass * const SpiClock = &(const struct ClockClass){
    .disable = spiClockDisable,
    .enable = spiClockEnable,
    .frequency = spiClockFrequency,
    .ready = spiClockReady
};

const struct ClockClass * const SystemPll = &(const struct ClockClass){
    .disable = sysPllDisable,
    .enable = sysPllEnable,
    .frequency = sysPllFrequency,
    .ready = sysPllReady
};

const struct ClockClass * const UartClock = &(const struct ClockClass){
    .disable = uartClockDisable,
    .enable = uartClockEnable,
    .frequency = uartClockFrequency,
    .ready = uartClockReady
};
/*----------------------------------------------------------------------------*/
static uint32_t extFrequency = 0;
uint32_t ticksPerSecond = TICK_RATE(INT_OSC_FREQUENCY);
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
static uint32_t rootClockFrequency(uint32_t cfg)
{
  uint32_t frequency = 0;

  switch (CLK_CFG0_ROOT_CLK_SEL_VALUE(cfg))
  {
    case ROOT_CLK_SEL_RC32M:
      frequency = INT_OSC_FREQUENCY;
      break;

    case ROOT_CLK_SEL_XTAL:
      frequency = extFrequency;
      break;

    default:
    {
      switch (CLK_CFG0_PLL_SEL_VALUE(cfg))
      {
        case PLL_SEL_48MHZ:
          frequency = 48000000;
          break;

        case PLL_SEL_120MHZ:
          frequency = 120000000;
          break;

        case PLL_SEL_160MHZ:
          frequency = 160000000;
          break;

        case PLL_SEL_192MHZ:
          frequency = 192000000;
          break;
      }
      break;
    }
  }

  return frequency;
}
/*----------------------------------------------------------------------------*/
static void extOscDisable(const void *)
{
  BL_AON->RF_TOP_AON &= ~(RF_TOP_AON_PU_XTAL_BUF_AON | RF_TOP_AON_PU_XTAL_AON);
  extFrequency = 0;
}
/*----------------------------------------------------------------------------*/
static enum Result extOscEnable(const void *, const void *configBase)
{
  const struct ExternalOscConfig * const config = configBase;
  assert(config != NULL);

  if (config->frequency != 24000000
      && config->frequency != 26000000
      && config->frequency != 32000000
      && config->frequency != 38400000
      && config->frequency != 40000000)
  {
    return E_VALUE;
  }

  BL_AON->RF_TOP_AON |= RF_TOP_AON_PU_XTAL_BUF_AON | RF_TOP_AON_PU_XTAL_AON;
  extFrequency = config->frequency;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t extOscFrequency(const void *)
{
  return (BL_AON->TSEN & TSEN_XTAL_RDY) ? extFrequency : 0;
}
/*----------------------------------------------------------------------------*/
static bool extOscReady(const void *)
{
  return (BL_AON->TSEN & TSEN_XTAL_RDY) != 0;
}
/*----------------------------------------------------------------------------*/
static void flashClockDisable(const void *)
{
  BL_GLB->CLK_CFG2 &= ~CLK_CFG2_SF_CLK_EN;
}
/*----------------------------------------------------------------------------*/
static enum Result flashClockEnable(const void *, const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > CLK_CFG2_SF_CLK_DIV_MAX)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t clkCfg2 = BL_GLB->CLK_CFG2;
  uint32_t frequency = 0;

  clkCfg2 &= ~(CLK_CFG2_SF_CLK_DIV_MASK | CLK_CFG2_SF_CLK_SEL_MASK);
  clkCfg2 |= CLK_CFG2_SF_CLK_DIV(divisor - 1);

  switch (config->source)
  {
    case CLOCK_PLL_80MHZ:
      clkCfg2 |= CLK_CFG2_SF_CLK_SEL(SF_CLK_SEL_80MHZ);
      frequency = 80000000;
      break;

    case CLOCK_PLL_96MHZ:
      clkCfg2 |= CLK_CFG2_SF_CLK_SEL(SF_CLK_SEL_96MHZ);
      frequency = 96000000;
      break;

    case CLOCK_PLL_120MHZ:
      clkCfg2 |= CLK_CFG2_SF_CLK_SEL(SF_CLK_SEL_120MHZ);
      frequency = 120000000;
      break;

    case CLOCK_SYSTEM:
      clkCfg2 |= CLK_CFG2_SF_CLK_SEL(SF_CLK_SEL_HCLK);
      frequency = mainClockFrequency(NULL);
      break;

    default:
      break;
  }

  if (!frequency)
    return E_VALUE;

  /* Reset access time */
  cacheControlAccess2T(true);
  /* Apply new clock source and divider */
  BL_GLB->CLK_CFG2 = clkCfg2;
  /* Configure access time depending on the new frequency */
  cacheControlAccess2T(frequency > 120000000);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t flashClockFrequency(const void *)
{
  const uint32_t clkCfg2 = BL_GLB->CLK_CFG2;
  const uint32_t divisor = CLK_CFG2_SF_CLK_DIV_VALUE(clkCfg2) + 1;
  uint32_t frequency;

  switch (CLK_CFG2_SF_CLK_SEL_VALUE(clkCfg2))
  {
    case SF_CLK_SEL_120MHZ:
      frequency = 120000000;
      break;

    case SF_CLK_SEL_80MHZ:
      frequency = 80000000;
      break;

    case SF_CLK_SEL_96MHZ:
      frequency = 96000000;
      break;

    default:
      frequency = mainClockFrequency(NULL);
      break;
  }

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static bool flashClockReady(const void *)
{
  return (BL_GLB->CLK_CFG2 & CLK_CFG2_SF_CLK_EN) != 0;
}
/*----------------------------------------------------------------------------*/
static void i2cClockDisable(const void *)
{
  BL_GLB->CLK_CFG3 &= ~CLK_CFG3_I2CEN;
}
/*----------------------------------------------------------------------------*/
static enum Result i2cClockEnable(const void *, const void *configBase)
{
  const struct DividedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > CLK_CFG3_I2CDIV_MAX)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t clkCfg3 = BL_GLB->CLK_CFG3;

  clkCfg3 &= ~CLK_CFG3_I2CDIV_MASK;
  clkCfg3 |= CLK_CFG3_I2CDIV(divisor - 1) | CLK_CFG3_I2CEN;

  BL_GLB->CLK_CFG3 = clkCfg3;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t i2cClockFrequency(const void *)
{
  const uint32_t clkCfg3 = BL_GLB->CLK_CFG3;

  if (clkCfg3 & CLK_CFG3_I2CEN)
  {
    const uint32_t divisor = CLK_CFG3_I2CDIV_VALUE(clkCfg3) + 1;
    const uint32_t frequency = socClockFrequency(NULL);

    return frequency / divisor;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static bool i2cClockReady(const void *)
{
  return (BL_GLB->CLK_CFG3 & CLK_CFG3_I2CEN) != 0;
}
/*----------------------------------------------------------------------------*/
static enum Result mainClockEnable(const void *, const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > CLK_CFG0_HCLK_DIV_MAX)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t clkCfg0 = BL_GLB->CLK_CFG0;
  uint32_t hbnGlb = BL_HBN->HBN_GLB;
  uint32_t frequency = 0;

  clkCfg0 &= ~CLK_CFG0_HCLK_DIV_MASK;
  clkCfg0 |= CLK_CFG0_HCLK_DIV(divisor - 1);
  hbnGlb &= ~HBN_GLB_ROOT_CLK_SEL_MASK;

  const uint32_t hbnGlbDefault =
      hbnGlb | HBN_GLB_ROOT_CLK_SEL(ROOT_CLK_SEL_RC32M);

  switch (config->source)
  {
    case CLOCK_INTERNAL:
      hbnGlb |= HBN_GLB_ROOT_CLK_SEL(ROOT_CLK_SEL_RC32M);
      frequency = INT_OSC_FREQUENCY;
      break;

    case CLOCK_EXTERNAL:
      hbnGlb |= HBN_GLB_ROOT_CLK_SEL(ROOT_CLK_SEL_XTAL);
      frequency = extFrequency;
      break;

    case CLOCK_PLL_48MHZ:
    case CLOCK_PLL_120MHZ:
    case CLOCK_PLL_160MHZ:
    case CLOCK_PLL_192MHZ:
    {
      if (!sysPllReady(NULL))
        return E_IDLE;

      clkCfg0 &= ~CLK_CFG0_PLL_SEL_MASK;

      switch (config->source)
      {
        case CLOCK_PLL_48MHZ:
          clkCfg0 |= CLK_CFG0_PLL_SEL(PLL_SEL_48MHZ);
          frequency = 48000000;
          break;

        case CLOCK_PLL_120MHZ:
          clkCfg0 |= CLK_CFG0_PLL_SEL(PLL_SEL_120MHZ);
          frequency = 120000000;
          break;

        case CLOCK_PLL_192MHZ:
          clkCfg0 |= CLK_CFG0_PLL_SEL(PLL_SEL_192MHZ);
          frequency = 192000000;
          break;

        default:
          clkCfg0 |= CLK_CFG0_PLL_SEL(PLL_SEL_160MHZ);
          frequency = 160000000;
          break;
      }

      hbnGlb |= HBN_GLB_ROOT_CLK_SEL(ROOT_CLK_SEL_PLL);
      break;
    }

    default:
      break;
  }

  if (!frequency)
    return E_VALUE;

  /* Switch to internal 32 MHz clock before configuration */
  BL_HBN->HBN_GLB = hbnGlbDefault;
  /* Reconfigure PLL settings and apply new clock source settings */
  BL_GLB->CLK_CFG0 = clkCfg0;
  BL_HBN->HBN_GLB = hbnGlb;

  ticksPerSecond = TICK_RATE(frequency / divisor);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t mainClockFrequency(const void *)
{
  const uint32_t clkCfg0 = BL_GLB->CLK_CFG0;
  const uint32_t divisor = CLK_CFG0_HCLK_DIV_VALUE(clkCfg0) + 1;
  const uint32_t frequency = rootClockFrequency(clkCfg0);

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static enum Result socClockEnable(const void *, const void *configBase)
{
  const struct DividedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > CLK_CFG0_BCLK_DIV_MAX)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t clkCfg0 = BL_GLB->CLK_CFG0;

  clkCfg0 &= ~CLK_CFG0_BCLK_DIV_MASK;
  clkCfg0 |= CLK_CFG0_BCLK_EN | CLK_CFG0_BCLK_DIV(divisor - 1);

  BL_GLB->CLK_CFG0 = clkCfg0 & ~CLK_CFG0_BCLK_EN;
  BL_GLB->CLK_CFG0 = clkCfg0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t socClockFrequency(const void *)
{
  const uint32_t clkCfg0 = BL_GLB->CLK_CFG0;
  const uint32_t divisor = CLK_CFG0_BCLK_DIV_VALUE(clkCfg0) + 1;
  const uint32_t frequency = rootClockFrequency(clkCfg0);

  return frequency / divisor;
}
/*----------------------------------------------------------------------------*/
static void spiClockDisable(const void *)
{
  BL_GLB->CLK_CFG3 &= ~CLK_CFG3_SPIEN;
}
/*----------------------------------------------------------------------------*/
static enum Result spiClockEnable(const void *, const void *configBase)
{
  const struct DividedClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > CLK_CFG3_SPIDIV_MAX)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t clkCfg3 = BL_GLB->CLK_CFG3;

  clkCfg3 &= ~CLK_CFG3_SPIDIV_MASK;
  clkCfg3 |= CLK_CFG3_SPIDIV(divisor - 1) | CLK_CFG3_SPIEN;

  BL_GLB->CLK_CFG3 = clkCfg3;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t spiClockFrequency(const void *)
{
  const uint32_t clkCfg3 = BL_GLB->CLK_CFG3;

  if (clkCfg3 & CLK_CFG3_SPIEN)
  {
    const uint32_t divisor = CLK_CFG3_SPIDIV_VALUE(clkCfg3) + 1;
    const uint32_t frequency = socClockFrequency(NULL);

    return frequency / divisor;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static bool spiClockReady(const void *)
{
  return (BL_GLB->CLK_CFG3 & CLK_CFG3_SPIEN) != 0;
}
/*----------------------------------------------------------------------------*/
static void sysPllDisable(const void *)
{
  BL_PDS->PU_RST_CLKPLL &= ~PU_RST_CLKPLL_ENABLE_MASK;
}
/*----------------------------------------------------------------------------*/
static enum Result sysPllEnable(const void *, const void *configBase)
{
  const struct PllConfig * const config = configBase;
  assert(config != NULL);
  assert(config->source == CLOCK_EXTERNAL || config->source == CLOCK_INTERNAL);

  if (config->source == CLOCK_INTERNAL)
  {
    BL_PDS->CLKPLL_TOP_CTRL =
        (BL_PDS->CLKPLL_TOP_CTRL & ~CLKPLL_TOP_CTRL_CLKPLL_REFCLK_SEL)
        | CLKPLL_TOP_CTRL_CLKPLL_XTAL_RC32M_SEL;
  }
  else
  {
    BL_PDS->CLKPLL_TOP_CTRL =
        (BL_PDS->CLKPLL_TOP_CTRL & ~CLKPLL_TOP_CTRL_CLKPLL_XTAL_RC32M_SEL)
        | CLKPLL_TOP_CTRL_CLKPLL_REFCLK_SEL;
  }

  /* Disable PLL */
  BL_PDS->PU_RST_CLKPLL &= ~PU_RST_CLKPLL_ENABLE_MASK;

  uint32_t clkPllCP = BL_PDS->CLKPLL_CP;
  uint32_t clkPllRZ = BL_PDS->CLKPLL_RZ;

  if (extFrequency == 26000000)
  {
    clkPllCP &= ~(CLKPLL_CP_CLKPLL_ICP_5U_MASK | CLKPLL_CP_CLKPLL_ICP_1U_MASK);
    clkPllCP |= CLKPLL_CP_CLKPLL_ICP_5U(0);
    clkPllCP |= CLKPLL_CP_CLKPLL_ICP_1U(1);
    clkPllCP |= CLKPLL_CP_CLKPLL_INT_FRAC_SW;

    clkPllRZ &= ~(CLKPLL_RZ_CLKPLL_C3_MASK | CLKPLL_RZ_CLKPLL_CZ_MASK
        | CLKPLL_RZ_CLKPLL_RZ_MASK);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_C3(2);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_CZ(2);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_RZ(5);
    clkPllRZ &= ~CLKPLL_RZ_CLKPLL_R4_SHORT;
  }
  else
  {
    clkPllCP &= ~(CLKPLL_CP_CLKPLL_ICP_5U_MASK | CLKPLL_CP_CLKPLL_ICP_1U_MASK);
    clkPllCP |= CLKPLL_CP_CLKPLL_ICP_5U(2);
    clkPllCP |= CLKPLL_CP_CLKPLL_ICP_1U(0);
    clkPllCP &= ~CLKPLL_CP_CLKPLL_INT_FRAC_SW;

    clkPllRZ &= ~(CLKPLL_RZ_CLKPLL_C3_MASK | CLKPLL_RZ_CLKPLL_CZ_MASK
        | CLKPLL_RZ_CLKPLL_RZ_MASK);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_C3(3);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_CZ(1);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_RZ(1);
    clkPllRZ |= CLKPLL_RZ_CLKPLL_R4_SHORT;
  }

  clkPllRZ &= ~CLKPLL_RZ_CLKPLL_R4_MASK;
  clkPllRZ |= CLKPLL_RZ_CLKPLL_R4(2);

  BL_PDS->CLKPLL_CP = clkPllCP;
  BL_PDS->CLKPLL_RZ = clkPllRZ;

  uint32_t clkPllTopCtrl = BL_PDS->CLKPLL_TOP_CTRL;

  clkPllTopCtrl &= ~(CLKPLL_TOP_CTRL_CLKPLL_POSTDIV_MASK
      | CLKPLL_TOP_CTRL_CLKPLL_REFDIV_RATIO_MASK);
  clkPllTopCtrl |= CLKPLL_TOP_CTRL_CLKPLL_POSTDIV(20);
  clkPllTopCtrl |= CLKPLL_TOP_CTRL_CLKPLL_REFDIV_RATIO(2);

  BL_PDS->CLKPLL_TOP_CTRL = clkPllTopCtrl;

  uint32_t clkPllSDM = BL_PDS->CLKPLL_SDM & ~CLKPLL_SDM_CLKPLL_SDMIN_MASK;

  if (config->source == CLOCK_EXTERNAL)
  {
    switch (extFrequency)
    {
      case 24000000:
        clkPllSDM |= CLKPLL_SDM_CLKPLL_SDMIN(0x500000);
        break;

      case 26000000:
        clkPllSDM |= CLKPLL_SDM_CLKPLL_SDMIN(0x49D89E);
        break;

      case 32000000:
        clkPllSDM |= CLKPLL_SDM_CLKPLL_SDMIN(0x3C0000);
        break;

      case 38400000:
        clkPllSDM |= CLKPLL_SDM_CLKPLL_SDMIN(0x320000);
        break;

      case 40000000:
        clkPllSDM |= CLKPLL_SDM_CLKPLL_SDMIN(0x300000);
        break;
    }
  }
  else
  {
    clkPllSDM |= CLKPLL_SDM_CLKPLL_SDMIN(0x3C0000);
  }

  BL_PDS->CLKPLL_SDM = clkPllSDM;

  uint32_t clkPllFBDV = BL_PDS->CLKPLL_FBDV;

  clkPllFBDV &= ~(CLKPLL_FBDV_CLKPLL_SEL_SAMPLE_CLK_MASK
      | CLKPLL_FBDV_CLKPLL_SEL_FB_CLK_MASK);
  clkPllFBDV |= CLKPLL_FBDV_CLKPLL_SEL_SAMPLE_CLK(1);
  clkPllFBDV |= CLKPLL_FBDV_CLKPLL_SEL_FB_CLK(1);

  BL_PDS->CLKPLL_FBDV = clkPllFBDV;

  /* PLL power-up sequence */

  uint32_t puRstClkPll = BL_PDS->PU_RST_CLKPLL;

  puRstClkPll |= PU_RST_CLKPLL_PU_CLKPLL_SFREG;
  BL_PDS->PU_RST_CLKPLL = puRstClkPll;
  udelay(5);

  puRstClkPll |= PU_RST_CLKPLL_ENABLE_MASK;
  BL_PDS->PU_RST_CLKPLL = puRstClkPll;
  udelay(5);

  puRstClkPll |= PU_RST_CLKPLL_CLKPLL_RESET_SDM;
  BL_PDS->PU_RST_CLKPLL = puRstClkPll;
  udelay(1);

  puRstClkPll |= PU_RST_CLKPLL_CLKPLL_RESET_FBDV;
  BL_PDS->PU_RST_CLKPLL = puRstClkPll;
  udelay(2);

  puRstClkPll &= ~PU_RST_CLKPLL_CLKPLL_RESET_FBDV;
  BL_PDS->PU_RST_CLKPLL = puRstClkPll;
  udelay(1);

  puRstClkPll &= ~PU_RST_CLKPLL_CLKPLL_RESET_SDM;
  BL_PDS->PU_RST_CLKPLL = puRstClkPll;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t sysPllFrequency(const void *)
{
  return 0;
}
/*----------------------------------------------------------------------------*/
static bool sysPllReady(const void *)
{
  return (BL_PDS->PU_RST_CLKPLL & PU_RST_CLKPLL_PU_CLKPLL) != 0;
}
/*----------------------------------------------------------------------------*/
static void uartClockDisable(const void *)
{
  BL_GLB->CLK_CFG2 &= ~CLK_CFG2_UART_CLK_EN;
}
/*----------------------------------------------------------------------------*/
static enum Result uartClockEnable(const void *, const void *configBase)
{
  const struct GenericClockConfig * const config = configBase;
  assert(config != NULL);

  if (config->divisor > CLK_CFG2_UART_CLK_DIV_MAX)
    return E_VALUE;

  const uint32_t divisor = config->divisor ? config->divisor : 1;
  uint32_t clkCfg2 = BL_GLB->CLK_CFG2;
  uint32_t hbnGlb = BL_HBN->HBN_GLB;

  clkCfg2 &= ~CLK_CFG2_UART_CLK_DIV_MASK;
  clkCfg2 |= CLK_CFG2_UART_CLK_DIV(divisor - 1) | CLK_CFG2_UART_CLK_EN;

  switch (config->source)
  {
    case CLOCK_SYSTEM:
      hbnGlb &= ~HBN_GLB_UART_CLK_SEL;
      break;

    case CLOCK_PLL_160MHZ:
      if (!sysPllReady(NULL))
        return E_IDLE;

      hbnGlb |= HBN_GLB_UART_CLK_SEL;
      break;

    default:
      return E_VALUE;
  }

  BL_GLB->CLK_CFG2 = clkCfg2 & ~CLK_CFG2_UART_CLK_EN;
  BL_HBN->HBN_GLB = hbnGlb;
  BL_GLB->CLK_CFG2 = clkCfg2;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static uint32_t uartClockFrequency(const void *)
{
  const uint32_t clkCfg2 = BL_GLB->CLK_CFG2;
  const uint32_t divisor = CLK_CFG2_UART_CLK_DIV_VALUE(clkCfg2) + 1;
  uint32_t frequency;

  if (clkCfg2 & CLK_CFG2_UART_CLK_EN)
  {
    if (clkCfg2 & HBN_GLB_UART_CLK_SEL)
      frequency = sysPllReady(NULL) ? 160000000 : 0;
    else
      frequency = mainClockFrequency(NULL);

    return frequency / divisor;
  }
  else
    return 0;
}
/*----------------------------------------------------------------------------*/
static bool uartClockReady(const void *)
{
  const uint32_t clkCfg2 = BL_GLB->CLK_CFG2;

  if (clkCfg2 & CLK_CFG2_UART_CLK_EN)
  {
    if (clkCfg2 & HBN_GLB_UART_CLK_SEL)
      return sysPllReady(NULL);
    else
      return true;
  }
  else
    return false;
}
