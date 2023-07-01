/*
 * sct_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/sct_base.h>
#include <halm/platform/lpc/sct_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/accel.h>
/*----------------------------------------------------------------------------*/
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
struct TimerHandler
{
  /* Timer descriptors */
  struct SctBase *parts[2];
  /* Free events */
  uint16_t events;
};
/*----------------------------------------------------------------------------*/
static bool timerHandlerActive(void);
static enum Result timerHandlerAttach(enum SctPart, struct SctBase *);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void timerHandlerDetach(enum SctPart);
#endif
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void tmrDeinit(void *);
#else
#define tmrDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SctBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = tmrInit,
    .deinit = tmrDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry sctInputPins[] = {
    {
        .key = PIN(PORT_1, 0), /* CTIN_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_1, 6), /* CTIN_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_2, 2), /* CTIN_6 */
        .channel = 0,
        .value = PACK_VALUE(5, 6)
    }, {
        .key = PIN(PORT_2, 3), /* CTIN_1 */
        .channel = 0,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = PIN(PORT_2, 4), /* CTIN_0 */
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = PIN(PORT_2, 5), /* CTIN_2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(PORT_2, 6), /* CTIN_7 */
        .channel = 0,
        .value = PACK_VALUE(5, 7)
    }, {
        .key = PIN(PORT_2, 13), /* CTIN_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_4, 8), /* CTIN_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_4, 9), /* CTIN_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_4, 10), /* CTIN_2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(PORT_6, 4), /* CTIN_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_7, 2), /* CTIN_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_7, 3), /* CTIN_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_B, 4), /* CTIN_5 */
        .channel = 0,
        .value = PACK_VALUE(5, 5)
    }, {
        .key = PIN(PORT_B, 5), /* CTIN_7 */
        .channel = 0,
        .value = PACK_VALUE(5, 7)
    }, {
        .key = PIN(PORT_B, 6), /* CTIN_6 */
        .channel = 0,
        .value = PACK_VALUE(5, 6)
    }, {
        .key = PIN(PORT_D, 7), /* CTIN_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_D, 8), /* CTIN_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_D, 10), /* CTIN_1 */
        .channel = 0,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_D, 13), /* CTIN_0 */
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(PORT_E, 9), /* CTIN_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_E, 10), /* CTIN_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_F, 8), /* CTIN_2 */
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry sctOutputPins[] = {
    {
        .key = PIN(PORT_1, 1), /* CTOUT_7 */
        .channel = 0,
        .value = PACK_VALUE(1, 7)
    }, {
        .key = PIN(PORT_1, 2), /* CTOUT_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_1, 3), /* CTOUT_8 */
        .channel = 0,
        .value = PACK_VALUE(1, 8)
    }, {
        .key = PIN(PORT_1, 4), /* CTOUT_9 */
        .channel = 0,
        .value = PACK_VALUE(1, 9)
    }, {
        .key = PIN(PORT_1, 5), /* CTOUT_10 */
        .channel = 0,
        .value = PACK_VALUE(1, 10)
    }, {
        .key = PIN(PORT_1, 7), /* CTOUT_13 */
        .channel = 0,
        .value = PACK_VALUE(2, 13)
    }, {
        .key = PIN(PORT_1, 8), /* CTOUT_12 */
        .channel = 0,
        .value = PACK_VALUE(2, 12)
    }, {
        .key = PIN(PORT_1, 9), /* CTOUT_11 */
        .channel = 0,
        .value = PACK_VALUE(2, 11)
    }, {
        .key = PIN(PORT_1, 10), /* CTOUT_14 */
        .channel = 0,
        .value = PACK_VALUE(2, 14)
    }, {
        .key = PIN(PORT_1, 11), /* CTOUT_15 */
        .channel = 0,
        .value = PACK_VALUE(2, 15)
    }, {
        .key = PIN(PORT_2, 7), /* CTOUT_1 */
        .channel = 0,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_2, 8), /* CTOUT_0 */
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(PORT_2, 9), /* CTOUT_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_2, 10), /* CTOUT_2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(PORT_2, 11), /* CTOUT_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_2, 12), /* CTOUT_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_4, 1), /* CTOUT_1 */
        .channel = 0,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_4, 2), /* CTOUT_0 */
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(PORT_4, 3), /* CTOUT_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_4, 4), /* CTOUT_2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(PORT_4, 5), /* CTOUT_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_4, 6), /* CTOUT_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_6, 5), /* CTOUT_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_6, 12), /* CTOUT_7 */
        .channel = 0,
        .value = PACK_VALUE(1, 7)
    }, {
        .key = PIN(PORT_7, 0), /* CTOUT_14 */
        .channel = 0,
        .value = PACK_VALUE(1, 14)
    }, {
        .key = PIN(PORT_7, 1), /* CTOUT_15 */
        .channel = 0,
        .value = PACK_VALUE(1, 15)
    }, {
        .key = PIN(PORT_7, 4), /* CTOUT_13 */
        .channel = 0,
        .value = PACK_VALUE(1, 13)
    }, {
        .key = PIN(PORT_7, 5), /* CTOUT_12 */
        .channel = 0,
        .value = PACK_VALUE(1, 12)
    }, {
        .key = PIN(PORT_7, 6), /* CTOUT_11 */
        .channel = 0,
        .value = PACK_VALUE(1, 11)
    }, {
        .key = PIN(PORT_7, 7), /* CTOUT_8 */
        .channel = 0,
        .value = PACK_VALUE(1, 8)
    }, {
        .key = PIN(PORT_A, 4), /* CTOUT_9 */
        .channel = 0,
        .value = PACK_VALUE(1, 9)
    }, {
        .key = PIN(PORT_B, 0), /* CTOUT_10 */
        .channel = 0,
        .value = PACK_VALUE(1, 10)
    }, {
        .key = PIN(PORT_B, 1), /* CTOUT_6 */
        .channel = 0,
        .value = PACK_VALUE(5, 6)
    }, {
        .key = PIN(PORT_B, 2), /* CTOUT_7 */
        .channel = 0,
        .value = PACK_VALUE(5, 7)
    }, {
        .key = PIN(PORT_B, 3), /* CTOUT_8 */
        .channel = 0,
        .value = PACK_VALUE(5, 8)
    }, {
        .key = PIN(PORT_D, 0), /* CTOUT_15 */
        .channel = 0,
        .value = PACK_VALUE(1, 15)
    }, {
        .key = PIN(PORT_D, 2), /* CTOUT_7 */
        .channel = 0,
        .value = PACK_VALUE(1, 7)
    }, {
        .key = PIN(PORT_D, 3), /* CTOUT_6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = PIN(PORT_D, 4), /* CTOUT_8 */
        .channel = 0,
        .value = PACK_VALUE(1, 8)
    }, {
        .key = PIN(PORT_D, 5), /* CTOUT_9 */
        .channel = 0,
        .value = PACK_VALUE(1, 9)
    }, {
        .key = PIN(PORT_D, 6), /* CTOUT_10 */
        .channel = 0,
        .value = PACK_VALUE(1, 10)
    }, {
        .key = PIN(PORT_D, 9), /* CTOUT_13 */
        .channel = 0,
        .value = PACK_VALUE(1, 13)
    }, {
        .key = PIN(PORT_D, 11), /* CTOUT_14 */
        .channel = 0,
        .value = PACK_VALUE(6, 14)
    }, {
        .key = PIN(PORT_D, 12), /* CTOUT_10 */
        .channel = 0,
        .value = PACK_VALUE(6, 10)
    }, {
        .key = PIN(PORT_D, 13), /* CTOUT_13 */
        .channel = 0,
        .value = PACK_VALUE(6, 13)
    }, {
        .key = PIN(PORT_D, 14), /* CTOUT_11 */
        .channel = 0,
        .value = PACK_VALUE(6, 11)
    }, {
        .key = PIN(PORT_D, 15), /* CTOUT_8 */
        .channel = 0,
        .value = PACK_VALUE(6, 8)
    }, {
        .key = PIN(PORT_D, 16), /* CTOUT_12 */
        .channel = 0,
        .value = PACK_VALUE(6, 12)
    }, {
        .key = PIN(PORT_E, 5), /* CTOUT_3 */
        .channel = 0,
        .value = PACK_VALUE(1, 3)
    }, {
        .key = PIN(PORT_E, 6), /* CTOUT_2 */
        .channel = 0,
        .value = PACK_VALUE(1, 2)
    }, {
        .key = PIN(PORT_E, 7), /* CTOUT_5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = PIN(PORT_E, 8), /* CTOUT_4 */
        .channel = 0,
        .value = PACK_VALUE(1, 4)
    }, {
        .key = PIN(PORT_E, 11), /* CTOUT_12 */
        .channel = 0,
        .value = PACK_VALUE(1, 12)
    }, {
        .key = PIN(PORT_E, 12), /* CTOUT_11 */
        .channel = 0,
        .value = PACK_VALUE(1, 11)
    }, {
        .key = PIN(PORT_E, 13), /* CTOUT_14 */
        .channel = 0,
        .value = PACK_VALUE(1, 14)
    }, {
        .key = PIN(PORT_E, 15), /* CTOUT_0 */
        .channel = 0,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = PIN(PORT_F, 9), /* CTOUT_1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct TimerHandler instance = {
    .parts = {NULL, NULL},
    .events = 0xFFFF
};
/*----------------------------------------------------------------------------*/
static bool timerHandlerActive(void)
{
  return instance.parts[0] != NULL || instance.parts[1] != NULL;
}
/*----------------------------------------------------------------------------*/
static enum Result timerHandlerAttach(enum SctPart timerPart,
    struct SctBase *timer)
{
  enum Result res = E_OK;

  if (timerPart == SCT_UNIFIED)
  {
    if (instance.parts[0] == NULL && instance.parts[1] == NULL)
      instance.parts[0] = timer;
    else
      res = E_BUSY;
  }
  else
  {
    const unsigned int part = timerPart == SCT_HIGH;

    // TODO Handle case when unified part already exists
    if (instance.parts[part] == NULL)
      instance.parts[part] = timer;
    else
      res = E_BUSY;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void timerHandlerDetach(enum SctPart timerPart)
{
  const unsigned int part = timerPart == SCT_HIGH;
  instance.parts[part] = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
void SCT_ISR(void)
{
  const uint16_t state = LPC_SCT->EVFLAG;

  if (instance.parts[0] != NULL && (instance.parts[0]->mask & state))
    instance.parts[0]->handler(instance.parts[0]);

  if (instance.parts[1] != NULL && (instance.parts[1]->mask & state))
    instance.parts[1]->handler(instance.parts[1]);

  /* Clear interrupt flags */
  LPC_SCT->EVFLAG = state;
}
/*----------------------------------------------------------------------------*/
bool sctAllocateEvent(struct SctBase *timer __attribute__((unused)),
    uint8_t *event)
{
  if (instance.events)
  {
    *event = 31 - countLeadingZeros32(instance.events);
    instance.events &= ~(1 << *event);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
uint32_t sctGetClock(const struct SctBase *timer __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void sctReleaseEvent(struct SctBase *timer __attribute__((unused)),
    uint8_t event)
{
  instance.events |= 1 << event;
}
/*----------------------------------------------------------------------------*/
static enum Result tmrInit(void *object, const void *configBase)
{
  const struct SctBaseConfig * const config = configBase;
  assert(config->edge == PIN_RISING || config->edge == PIN_FALLING);
  assert(config->input < SCT_INPUT_END);

  struct SctBase * const timer = object;
  uint32_t value = 0;

  /* Check whether the timer is divided into two separate parts */
  if (config->part == SCT_UNIFIED)
    value |= CONFIG_UNIFY;

  /* Configure timer clock source */
  if (config->input != SCT_INPUT_NONE)
  {
    value |= CONFIG_CLKMODE(CLKMODE_INPUT_HP);

    if (config->edge == PIN_RISING)
      value |= CONFIG_CKSEL_RISING(config->input - 1);
    else
      value |= CONFIG_CKSEL_FALLING(config->input - 1);
  }

  const bool enabled = timerHandlerActive();
  LPC_SCT_Type * const reg = LPC_SCT;

  if (enabled)
  {
    /*
     * Compare current timer configuration with proposed one
     * if the timer is already enabled.
     */
    if (value != (reg->CONFIG & CONFIG_SHARED_MASK))
      return E_BUSY;
  }

  const enum Result res = timerHandlerAttach(config->part, timer);

  if (res == E_OK)
  {
    timer->channel = config->channel;
    timer->handler = NULL;
    timer->irq = SCT_IRQ;
    timer->mask = 0;
    timer->part = config->part;
    timer->reg = reg;

    if (!enabled)
    {
      /* Enable clock to peripheral */
      sysClockEnable(CLK_M4_SCT);
      /* Reset registers to default values */
      sysResetEnable(RST_SCT);
      /* Enable common interrupt */
      irqEnable(timer->irq);
    }

    reg->CONFIG = (reg->CONFIG & ~CONFIG_SHARED_MASK) | value;
  }

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void tmrDeinit(void *object)
{
  struct SctBase * const timer = object;

  timerHandlerDetach(timer->part);

  if (!timerHandlerActive())
  {
    /* Disable common interrupt */
    irqDisable(timer->irq);
    /* Disable clock when all timer parts are inactive */
    sysClockDisable(CLK_M4_SCT);
  }
}
#endif
