/*
 * gptimer_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/gptimer_base.h>
#include <halm/platform/numicro/gptimer_defs.h>
#include <halm/platform/numicro/system.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry gpTimerPins[] = {
    {
        .key = PIN(PORT_A, 6), /* TM3 */
        .channel = 3,
        .value = 14
    }, {
        .key = PIN(PORT_A, 7), /* TM2 */
        .channel = 2,
        .value = 14
    }, {
        .key = PIN(PORT_A, 8), /* TM3_EXT */
        .channel = 3,
        .value = 13
    }, {
        .key = PIN(PORT_A, 9), /* TM2_EXT */
        .channel = 2,
        .value = 13
    }, {
        .key = PIN(PORT_A, 10), /* TM1_EXT */
        .channel = 1,
        .value = 13
    }, {
        .key = PIN(PORT_A, 11), /* TM0_EXT */
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_B, 2), /* TM3 */
        .channel = 3,
        .value = 14
    }, {
        .key = PIN(PORT_B, 3), /* TM2 */
        .channel = 2,
        .value = 14
    }, {
        .key = PIN(PORT_B, 4), /* TM1 */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_B, 5), /* TM0 */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_B, 12), /* TM3_EXT */
        .channel = 3,
        .value = 13
    }, {
        .key = PIN(PORT_B, 13), /* TM2_EXT */
        .channel = 2,
        .value = 13
    }, {
        .key = PIN(PORT_B, 14), /* TM1_EXT */
        .channel = 1,
        .value = 13
    }, {
        .key = PIN(PORT_B, 15), /* TM0_EXT */
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_C, 6), /* TM1 */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_C, 7), /* TM0 */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_C, 14), /* TM1 */
        .channel = 1,
        .value = 13
    }, {
        .key = PIN(PORT_D, 0), /* TM2 */
        .channel = 2,
        .value = 14
    }, {
        .key = PIN(PORT_F, 11), /* TM3 */
        .channel = 3,
        .value = 13
    }, {
        .key = PIN(PORT_G, 2), /* TM0 */
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_G, 3), /* TM1 */
        .channel = 1,
        .value = 13
    }, {
        .key = PIN(PORT_G, 4), /* TM2 */
        .channel = 2,
        .value = 13
    }, {
        .key = PIN(PORT_H, 0), /* TM0_EXT on M48xxIDAE */
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_H, 1), /* TM1_EXT on M48xxIDAE */
        .channel = 1,
        .value = 13
    }, {
        .key = PIN(PORT_H, 2), /* TM2_EXT on M48xxIDAE */
        .channel = 2,
        .value = 13
    }, {
        .key = PIN(PORT_H, 3), /* TM3_EXT on M48xxIDAE */
        .channel = 3,
        .value = 13
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[4] = {0};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct GpTimerBase *object)
{
  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void TMR0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void TMR1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void TMR2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void TMR3_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(const struct GpTimerBase *timer)
{
  const void *clock = NULL;

  switch (timer->channel)
  {
    case 0:
      clock = Timer0Clock;
      break;

    case 1:
      clock = Timer1Clock;
      break;

    case 2:
      clock = Timer2Clock;
      break;

    case 3:
      clock = Timer3Clock;
      break;
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  static NM_TIMER_Type * const TIMER_BLOCKS[] = {
      NM_TIMER0, NM_TIMER1, NM_TIMER2, NM_TIMER3
  };

  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;

  if (!setInstance(config->channel, timer))
    return E_BUSY;

  timer->channel = config->channel;
  timer->handler = NULL;
  timer->irq = TMR0_IRQ + config->channel;
  timer->reg = TIMER_BLOCKS[config->channel];

  sysClockEnable(CLK_TMR0 + config->channel);
  sysResetBlock(RST_TMR0 + config->channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;

  sysClockDisable(CLK_TMR0 + timer->channel);
  instances[timer->channel] = NULL;
}
#endif
