/*
 * sdh_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/m48x/clocking_defs.h>
#include <halm/platform/numicro/sdh_base.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(struct SdhBase *, const struct SdhBaseConfig *);
static bool setInstance(uint8_t, struct SdhBase *);
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_SDH_NO_DEINIT
static void sdioDeinit(void *);
#else
#  define sdioDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SdhBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = sdioInit,
    .deinit = sdioDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry sdhPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_SDH0
    /* SD0_CLK */
    {
        .key = PIN(PORT_B, 1), /* SD0_CLK */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 6), /* SD0_CLK */
        .channel = 0,
        .value = 3
    },
    /* SD0_CMD */
    {
        .key = PIN(PORT_B, 0), /* SD0_CMD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 7), /* SD0_CMD */
        .channel = 0,
        .value = 3
    },
    /* SD0_DAT0 */
    {
        .key = PIN(PORT_B, 2), /* SD0_DAT0 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 2), /* SD0_DAT0 */
        .channel = 0,
        .value = 3
    },
    /* SD0_DAT1 */
    {
        .key = PIN(PORT_B, 3), /* SD0_DAT1 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 3), /* SD0_DAT1 */
        .channel = 0,
        .value = 3
    },
    /* SD0_DAT2 */
    {
        .key = PIN(PORT_B, 4), /* SD0_DAT2 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 4), /* SD0_DAT2 */
        .channel = 0,
        .value = 3
    },
    /* SD0_DAT3 */
    {
        .key = PIN(PORT_B, 5), /* SD0_DAT3 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_E, 5), /* SD0_DAT3 */
        .channel = 0,
        .value = 3
    },
    /* SD0_nCD */
    {
        .key = PIN(PORT_B, 12), /* SD0_nCD */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_D, 13), /* SD0_nCD */
        .channel = 0,
        .value = 3
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_SDH1
    /* SD1_CLK */
    {
        .key = PIN(PORT_A, 4), /* SD1_CLK */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 6), /* SD1_CLK */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_G, 14), /* SD1_CLK */
        .channel = 1,
        .value = 3
    },
    /* SD1_CMD */
    {
        .key = PIN(PORT_A, 5), /* SD1_CMD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_B, 7), /* SD1_CMD */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_G, 13), /* SD1_CMD */
        .channel = 1,
        .value = 3
    },
    /* SD1_DAT0 */
    {
        .key = PIN(PORT_A, 0), /* SD1_DAT0 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_A, 8), /* SD1_DAT0 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_G, 12), /* SD1_DAT0 */
        .channel = 1,
        .value = 3
    },
    /* SD1_DAT1 */
    {
        .key = PIN(PORT_A, 1), /* SD1_DAT1 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_A, 9), /* SD1_DAT1 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_G, 11), /* SD1_DAT1 */
        .channel = 1,
        .value = 3
    },
    /* SD1_DAT2 */
    {
        .key = PIN(PORT_A, 2), /* SD1_DAT2 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_A, 10), /* SD1_DAT2 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_G, 10), /* SD1_DAT2 */
        .channel = 1,
        .value = 3
    },
    /* SD1_DAT3 */
    {
        .key = PIN(PORT_A, 3), /* SD1_DAT3 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_A, 11), /* SD1_DAT3 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_G, 9), /* SD1_DAT2 */
        .channel = 1,
        .value = 3
    },
    /* SD1_nCD */
    {
        .key = PIN(PORT_A, 6), /* SD1_nCD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 14), /* SD1_nCD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_G, 15), /* SD1_nCD */
        .channel = 1,
        .value = 3
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct SdhBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(struct SdhBase *interface,
    const struct SdhBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->clk,
      config->cmd,
      config->dat0,
      config->dat1,
      config->dat2,
      config->dat3
  };
  bool wide = true;

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (!pinArray[index])
    {
      /* First three pins are mandatory */
      assert(index >= 3);

      wide = false;
      continue;
    }

    const struct PinEntry * const pinEntry = pinFind(sdhPins,
        pinArray[index], 0);
    assert(pinEntry);

    const struct Pin pin = pinInit(pinArray[index]);

    pinInput(pin);
    pinSetFunction(pin, pinEntry->value);
  }

  interface->wide = wide;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct SdhBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SDH0
void SDHOST0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_SDH1
void SDHOST1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t sdhGetClock(const struct SdhBase *interface)
{
  return clockFrequency(interface->channel ? Sdh1Clock : Sdh0Clock);
}
/*----------------------------------------------------------------------------*/
uint32_t sdhGetDivider(const struct SdhBase *interface)
{
  const enum ClockDivider divider = interface->channel ?
      DIVIDER_SDH1 : DIVIDER_SDH0;

  const uint32_t index = EXTRACT_DIVIDER_INDEX(divider);
  const uint32_t offset = EXTRACT_DIVIDER_OFFSET(divider);
  const uint32_t mask = BIT_FIELD(MASK(EXTRACT_DIVIDER_SIZE(divider)), offset);

  return ((NM_CLK->CLKDIV[index] & mask) >> offset) + 1;
}
/*----------------------------------------------------------------------------*/
void sdhSetDivider(struct SdhBase *interface, uint32_t value)
{
  const enum ClockDivider divider = interface->channel ?
      DIVIDER_SDH1 : DIVIDER_SDH0;

  const uint32_t index = EXTRACT_DIVIDER_INDEX(divider);
  const uint32_t offset = EXTRACT_DIVIDER_OFFSET(divider);
  const uint32_t max = MASK(EXTRACT_DIVIDER_SIZE(divider));
  uint32_t clkdiv;

  --value;
  if (value > max)
    value = max;

  clkdiv = NM_CLK->CLKDIV[index];
  clkdiv &= ~BIT_FIELD(max, offset);
  clkdiv |= BIT_FIELD(value, offset);
  NM_CLK->CLKDIV[index] = clkdiv;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdhBaseConfig * const config = configBase;
  struct SdhBase * const interface = object;
  enum SysClockBranch branch;
  enum SysBlockReset reset;

  switch (config->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_SDH0
    case 0:
      branch = CLK_SDH0;
      reset = RST_SDH0;

      interface->irq = SDHOST0_IRQ;
      interface->reg = NM_SDH0;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_SDH1
    case 1:
      branch = CLK_SDH1;
      reset = RST_SDH1;

      interface->irq = SDHOST1_IRQ;
      interface->reg = NM_SDH1;
      break;
#endif

    default:
      return E_VALUE;
  }

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  configPins(interface, config);

  /* Enable clock to peripheral */
  sysClockEnable(branch);
  /* Reset registers to default values */
  sysResetBlock(reset);

  interface->channel = config->channel;
  interface->handler = NULL;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_SDH_NO_DEINIT
static void sdioDeinit(void *object)
{
  const struct SdhBase * const interface = object;

  sysClockDisable(interface->channel ? CLK_SDH1 : CLK_SDH0);
  instances[interface->channel] = NULL;
}
#endif
