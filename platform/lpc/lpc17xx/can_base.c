/*
 * can_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_1/can_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static void configPins(const struct CanBaseConfig *);
static bool setInstance(uint8_t, struct CanBase *);
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_CAN_NO_DEINIT
static void canDeinit(void *);
#else
#  define canDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const CanBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = canInit,
    .deinit = canDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry canPins[] = {
    {
        .key = PIN(0, 0), /* RD1 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 1), /* TD1 */
        .channel = 0,
        .value = 1
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 4), /* RD2 */
        .channel = 1,
        .value = 2
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 5), /* TD2 */
        .channel = 1,
        .value = 2
    }, {
        /* Unavailable on LPC175x series */
        .key = PIN(0, 21), /* RD1 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(0, 22), /* TD1 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(2, 7), /* RD2 */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(2, 8), /* TD2 */
        .channel = 1,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct CanBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static void configPins(const struct CanBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Configure RX pin */
  if (config->rx)
  {
    pinEntry = pinFind(canPins, config->rx, config->channel);
    assert(pinEntry != NULL);
    pinInput((pin = pinInit(config->rx)));
    pinSetFunction(pin, pinEntry->value);
  }

  /* Configure TX pin */
  if (config->tx)
  {
    pinEntry = pinFind(canPins, config->tx, config->channel);
    assert(pinEntry != NULL);
    pinInput((pin = pinInit(config->tx)));
    pinSetFunction(pin, pinEntry->value);
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct CanBase *object)
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
void CAN_ISR(void)
{
  if (instances[0] != NULL)
    instances[0]->handler(instances[0]);

  if (instances[1] != NULL)
    instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct CanBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct CanBaseConfig * const config = configBase;
  struct CanBase * const interface = object;

  /*
   * Disable interrupt requests during initialization, because the same vector
   * is used for both CAN modules. Interrupt requests will be enabled in
   * upper-level code after completion of initialization.
   */
  const bool irqEnabled = irqStatus(CAN_IRQ);
  irqDisable(CAN_IRQ);

  if (!setInstance(config->channel, interface))
  {
    if (irqEnabled)
      irqEnable(CAN_IRQ);
    return E_BUSY;
  }

  /* Configure input and output pins */
  configPins(config);

  switch (config->channel)
  {
    case 0:
      sysPowerEnable(PWR_CAN1);
      interface->irq = CAN_IRQ;
      interface->reg = LPC_CAN1;
      break;

    case 1:
      sysPowerEnable(PWR_CAN2);
      interface->irq = CAN_IRQ;
      interface->reg = LPC_CAN2;
      break;
  }

  sysClockControl(CLK_CAN1, DEFAULT_DIV);
  sysClockControl(CLK_CAN2, DEFAULT_DIV);
  sysClockControl(CLK_ACF, DEFAULT_DIV);

  interface->channel = config->channel;
  interface->handler = NULL;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_CAN_NO_DEINIT
static void canDeinit(void *object)
{
  const struct CanBase * const interface = object;

  switch (interface->channel)
  {
    case 0:
      sysPowerDisable(PWR_CAN1);
      break;

    case 1:
      sysPowerDisable(PWR_CAN2);
      break;
  }

  instances[interface->channel] = NULL;

  /* Re-enable IRQ when the second module is still used */
  if (instances[interface->channel ^ 1] != NULL)
    irqEnable(CAN_IRQ);
}
#endif
