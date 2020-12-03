/*
 * can_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/can_base.h>
#include <halm/platform/lpc/gen_1/can_defs.h>
#include <halm/platform/lpc/lpc17xx/clocking.h>
#include <halm/platform/lpc/lpc17xx/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static void configPins(const struct CanBase *, const struct CanBaseConfig *);
static bool setInstance(uint8_t, struct CanBase *);
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_CAN_NO_DEINIT
static void canDeinit(void *);
#else
#define canDeinit deletedDestructorTrap
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
static struct CanBase *instances[2] = {0};
/*----------------------------------------------------------------------------*/
static void configPins(const struct CanBase *interface,
    const struct CanBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  /* Configure RX pin */
  pinEntry = pinFind(canPins, config->rx, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->rx)));
  pinSetFunction(pin, pinEntry->value);

  /* Configure TX pin */
  pinEntry = pinFind(canPins, config->tx, interface->channel);
  assert(pinEntry);
  pinInput((pin = pinInit(config->tx)));
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct CanBase *object)
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
void CAN_ISR(void)
{
  if (instances[0])
    instances[0]->handler(instances[0]);

  if (instances[1])
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

  interface->channel = config->channel;
  interface->handler = 0;

  /*
   * Disable interrupt requests during initialization, because the same vector
   * is used for both CAN modules. Interrupt requests will be enabled in
   * upper-level code after completion of initialization.
   */
  const bool irqEnabled = irqStatus(CAN_IRQ);
  irqDisable(CAN_IRQ);

  if (!setInstance(interface->channel, interface))
  {
    if (irqEnabled)
      irqEnable(CAN_IRQ);
    return E_BUSY;
  }

  /* Configure input and output pins */
  configPins(interface, config);

  switch (interface->channel)
  {
    case 0:
      sysPowerEnable(PWR_CAN1);
      interface->reg = LPC_CAN1;
      interface->irq = CAN_IRQ;
      break;

    case 1:
      sysPowerEnable(PWR_CAN2);
      interface->reg = LPC_CAN2;
      interface->irq = CAN_IRQ;
      break;
  }

  sysClockControl(CLK_CAN1, DEFAULT_DIV);
  sysClockControl(CLK_CAN2, DEFAULT_DIV);
  sysClockControl(CLK_ACF, DEFAULT_DIV);

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

  instances[interface->channel] = 0;

  /* Re-enable IRQ for the second module if it is still active */
  if (instances[!interface->channel])
    irqDisable(CAN_IRQ);
}
#endif
