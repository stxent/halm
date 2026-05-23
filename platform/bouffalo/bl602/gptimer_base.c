/*
 * gptimer_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/clocking.h>
#include <halm/platform/bouffalo/gptimer_defs.h>
#include <halm/platform/bouffalo/gptimer_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct GpTimerBase *);
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_BOUFFALO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *);
#else
#  define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const GpTimerBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
static struct GpTimerBase *instances[3] = {NULL};
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
[[gnu::interrupt]] void TIMER_CH0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void TIMER_CH1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void TIMER_WDT_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
uint32_t gpTimerGetClock(const struct GpTimerBase *timer)
{
  const uint32_t source = TCCR_CS_VALUE(timer->channel, BL_TIMER->TCCR);

  switch (source)
  {
    case CS_FCLK:
      return clockFrequency(MainClock);

    default:
      // TODO
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct GpTimerBaseConfig * const config = configBase;
  struct GpTimerBase * const timer = object;

  if (!setInstance(config->channel, timer))
    return E_BUSY;

  timer->channel = config->channel;
  timer->handler = NULL;
  timer->irq = TIMER_CH0_IRQ + config->channel;

  /* Configure timer clock input */
  BL_TIMER->TCCR = (BL_TIMER->TCCR & TCCR_CS_MASK(timer->channel))
      | TCCR_CS(timer->channel, CS_FCLK);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_GPTIMER_NO_DEINIT
static void tmrDeinit(void *object)
{
  const struct GpTimerBase * const timer = object;
  instances[timer->channel] = NULL;
}
#endif
