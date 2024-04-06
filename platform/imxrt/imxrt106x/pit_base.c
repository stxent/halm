/*
 * pit_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/clocking.h>
#include <halm/platform/imxrt/pit_base.h>
#include <halm/platform/imxrt/pit_defs.h>
#include <halm/platform/imxrt/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct PitBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_IMXRT_PIT_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const PitBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry pitPins[] = {
    {
        .key = PIN(PORT_AD_B0, 4), /* PIT_TRIGGER00 */
        .channel = 0,
        .value = 6
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct PitBase *instances[4] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct PitBase *object)
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
void PIT_ISR(void)
{
  if (instances[0] != NULL)
    instances[0]->handler(instances[0]);

  if (instances[1] != NULL)
    instances[1]->handler(instances[1]);

  if (instances[2] != NULL)
    instances[2]->handler(instances[2]);

  if (instances[3] != NULL)
    instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t pitGetClock(const struct PitBase *)
{
  return clockFrequency(TimerClock);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct PitBaseConfig * const config = configBase;
  struct PitBase * const interface = object;

  assert(!config->chain || (config->channel < ARRAY_SIZE(instances) - 1));

  if (!setInstance(config->channel, interface))
    return E_BUSY;
  if (config->chain && !setInstance(config->channel + 1, interface))
  {
    instances[config->channel] = NULL;
    return E_BUSY;
  }

  interface->chain = config->chain;
  interface->channel = config->channel;
  interface->counter = config->chain ? config->channel + 1 : config->channel;
  interface->handler = NULL;
  interface->irq = PIT_IRQ;
  interface->reg = IMX_PIT;

  if (!sysClockStatus(CLK_PIT))
  {
    /* Disable clock gating */
    sysClockEnable(CLK_PIT);

    /* Enable internal clock for timers */
    IMX_PIT->MCR = 0;

    /* Enable interrupts for all channels */
    irqEnable(PIT_IRQ);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_IMXRT_PIT_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct PitBase * const interface = object;
  bool active = false;

  instances[interface->channel] = NULL;
  if (interface->chain)
    instances[interface->channel + 1] = NULL;

  for (size_t index = 0; index < ARRAY_SIZE(instances); ++index)
  {
    if (instances[index] != NULL)
    {
      active = true;
      break;
    }
  }

  if (!active)
  {
    irqDisable(PIT_IRQ);
    IMX_PIT->MCR = MCR_MDIS;
    sysClockDisable(CLK_PIT);
  }
}
#endif
