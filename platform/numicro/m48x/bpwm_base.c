/*
 * bpwm_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/bpwm_base.h>
#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* Pack match channel and pin function in one value */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct BpwmUnitBase *);
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const BpwmUnitBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry bpwmPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM0
    /* BPWM0_CH0 */
    {
        .key = PIN(PORT_A, 0), /* BPWM0_CH0 */
        .channel = 0,
        .value = PACK_VALUE(12, 0)
    }, {
        .key = PIN(PORT_A, 11), /* BPWM0_CH0 */
        .channel = 0,
        .value = PACK_VALUE(9, 0)
    }, {
        .key = PIN(PORT_E, 2), /* BPWM0_CH0 */
        .channel = 0,
        .value = PACK_VALUE(13, 0)
    }, {
        .key = PIN(PORT_G, 14), /* BPWM0_CH0 */
        .channel = 0,
        .value = PACK_VALUE(12, 0)
    },
    /* BPWM0_CH1 */
    {
        .key = PIN(PORT_A, 1), /* BPWM0_CH1 */
        .channel = 0,
        .value = PACK_VALUE(12, 1)
    }, {
        .key = PIN(PORT_A, 10), /* BPWM0_CH1 */
        .channel = 0,
        .value = PACK_VALUE(9, 1)
    }, {
        .key = PIN(PORT_E, 3), /* BPWM0_CH1 */
        .channel = 0,
        .value = PACK_VALUE(13, 1)
    }, {
        .key = PIN(PORT_G, 13), /* BPWM0_CH1 */
        .channel = 0,
        .value = PACK_VALUE(12, 1)
    },
    /* BPWM0_CH2 */
    {
        .key = PIN(PORT_A, 2), /* BPWM0_CH2 */
        .channel = 0,
        .value = PACK_VALUE(12, 2)
    }, {
        .key = PIN(PORT_A, 9), /* BPWM0_CH2 */
        .channel = 0,
        .value = PACK_VALUE(9, 2)
    }, {
        .key = PIN(PORT_E, 4), /* BPWM0_CH2 */
        .channel = 0,
        .value = PACK_VALUE(13, 2)
    }, {
        .key = PIN(PORT_G, 12), /* BPWM0_CH2 */
        .channel = 0,
        .value = PACK_VALUE(12, 2)
    },
    /* BPWM0_CH3 */
    {
        .key = PIN(PORT_A, 3), /* BPWM0_CH3 */
        .channel = 0,
        .value = PACK_VALUE(12, 3)
    }, {
        .key = PIN(PORT_A, 8), /* BPWM0_CH3 */
        .channel = 0,
        .value = PACK_VALUE(9, 3)
    }, {
        .key = PIN(PORT_E, 5), /* BPWM0_CH3 */
        .channel = 0,
        .value = PACK_VALUE(13, 3)
    }, {
        .key = PIN(PORT_G, 11), /* BPWM0_CH3 */
        .channel = 0,
        .value = PACK_VALUE(12, 3)
    },
    /* BPWM0_CH4 */
    {
        .key = PIN(PORT_A, 4), /* BPWM0_CH4 */
        .channel = 0,
        .value = PACK_VALUE(12, 4)
    }, {
        .key = PIN(PORT_C, 13), /* BPWM0_CH4 */
        .channel = 0,
        .value = PACK_VALUE(9, 4)
    }, {
        .key = PIN(PORT_E, 6), /* BPWM0_CH4 */
        .channel = 0,
        .value = PACK_VALUE(13, 4)
    }, {
        .key = PIN(PORT_F, 5), /* BPWM0_CH4 */
        .channel = 0,
        .value = PACK_VALUE(8, 4)
    }, {
        .key = PIN(PORT_G, 10), /* BPWM0_CH4 */
        .channel = 0,
        .value = PACK_VALUE(12, 4)
    },
    /* BPWM0_CH5 */
    {
        .key = PIN(PORT_A, 5), /* BPWM0_CH5 */
        .channel = 0,
        .value = PACK_VALUE(12, 5)
    }, {
        .key = PIN(PORT_D, 12), /* BPWM0_CH5 */
        .channel = 0,
        .value = PACK_VALUE(9, 5)
    }, {
        .key = PIN(PORT_E, 7), /* BPWM0_CH5 */
        .channel = 0,
        .value = PACK_VALUE(13, 5)
    }, {
        .key = PIN(PORT_F, 4), /* BPWM0_CH5 */
        .channel = 0,
        .value = PACK_VALUE(8, 5)
    }, {
        .key = PIN(PORT_G, 9), /* BPWM0_CH5 */
        .channel = 0,
        .value = PACK_VALUE(12, 5)
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM1
    /* BPWM1_CH0 */
    {
        .key = PIN(PORT_B, 11), /* BPWM1_CH0 */
        .channel = 1,
        .value = PACK_VALUE(10, 0)
    }, {
        .key = PIN(PORT_C, 7), /* BPWM1_CH0 */
        .channel = 1,
        .value = PACK_VALUE(12, 0)
    }, {
        .key = PIN(PORT_F, 0), /* BPWM1_CH0 */
        .channel = 1,
        .value = PACK_VALUE(12, 0)
    }, {
        .key = PIN(PORT_F, 3), /* BPWM1_CH0 */
        .channel = 1,
        .value = PACK_VALUE(11, 0)
    },
    /* BPWM1_CH1 */
    {
        .key = PIN(PORT_B, 10), /* BPWM1_CH1 */
        .channel = 1,
        .value = PACK_VALUE(10, 1)
    }, {
        .key = PIN(PORT_C, 6), /* BPWM1_CH1 */
        .channel = 1,
        .value = PACK_VALUE(12, 1)
    }, {
        .key = PIN(PORT_F, 1), /* BPWM1_CH1 */
        .channel = 1,
        .value = PACK_VALUE(12, 1)
    }, {
        .key = PIN(PORT_F, 2), /* BPWM1_CH1 */
        .channel = 1,
        .value = PACK_VALUE(11, 1)
    },
    /* BPWM1_CH2 */
    {
        .key = PIN(PORT_A, 7), /* BPWM1_CH2 */
        .channel = 1,
        .value = PACK_VALUE(12, 2)
    }, {
        .key = PIN(PORT_A, 12), /* BPWM1_CH2 */
        .channel = 1,
        .value = PACK_VALUE(11, 2)
    }, {
        .key = PIN(PORT_B, 9), /* BPWM1_CH2 */
        .channel = 1,
        .value = PACK_VALUE(10, 2)
    },
    /* BPWM1_CH3 */
    {
        .key = PIN(PORT_A, 6), /* BPWM1_CH3 */
        .channel = 1,
        .value = PACK_VALUE(12, 3)
    }, {
        .key = PIN(PORT_A, 13), /* BPWM1_CH3 */
        .channel = 1,
        .value = PACK_VALUE(11, 3)
    }, {
        .key = PIN(PORT_B, 8), /* BPWM1_CH3 */
        .channel = 1,
        .value = PACK_VALUE(10, 3)
    },
    /* BPWM1_CH4 */
    {
        .key = PIN(PORT_A, 14), /* BPWM1_CH4 */
        .channel = 1,
        .value = PACK_VALUE(11, 4)
    }, {
        .key = PIN(PORT_B, 7), /* BPWM1_CH4 */
        .channel = 1,
        .value = PACK_VALUE(10, 4)
    }, {
        .key = PIN(PORT_C, 8), /* BPWM1_CH4 */
        .channel = 1,
        .value = PACK_VALUE(12, 4)
    },
    /* BPWM1_CH5 */
    {
        .key = PIN(PORT_A, 15), /* BPWM1_CH5 */
        .channel = 1,
        .value = PACK_VALUE(11, 5)
    }, {
        .key = PIN(PORT_B, 6), /* BPWM1_CH5 */
        .channel = 1,
        .value = PACK_VALUE(10, 5)
    }, {
        .key = PIN(PORT_E, 13), /* BPWM1_CH5 */
        .channel = 1,
        .value = PACK_VALUE(12, 5)
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct BpwmUnitBase *instances[2] = {0};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct BpwmUnitBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (!instances[channel])
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM0
void BPWM0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM1
void BPWM1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t bpwmGetClock(const struct BpwmUnitBase *unit)
{
  const void *clock = 0;

  switch (unit->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM0
    case 0:
      clock = Bpwm0Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_BPWM1
    case 1:
      clock = Bpwm1Clock;
      break;
#endif
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object, const void *configBase)
{
  const struct BpwmUnitBaseConfig * const config = configBase;
  struct BpwmUnitBase * const unit = object;
  enum SysClockBranch branch;
  enum SysBlockReset reset;

  switch (config->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_BPWM0
    case 0:
      branch = CLK_BPWM0;
      reset = RST_BPWM0;

      unit->irq = BPWM0_IRQ;
      unit->reg = NM_BPWM0;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_BPWM1
    case 1:
      branch = CLK_BPWM1;
      reset = RST_BPWM1;

      unit->irq = BPWM1_IRQ;
      unit->reg = NM_BPWM1;
      break;
#endif

    default:
      return E_VALUE;
  }

  if (!setInstance(unit->channel, unit))
    return E_BUSY;

  /* Enable clock to peripheral */
  sysClockEnable(branch);
  /* Reset registers to default values */
  sysResetBlock(reset);

  unit->channel = config->channel;
  unit->handler = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_BPWM_NO_DEINIT
static void unitDeinit(void *object)
{
  const struct BpwmUnitBase * const unit = object;
  sysClockDisable(unit->channel ? CLK_BPWM1 : CLK_BPWM0);
}
#endif
