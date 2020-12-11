/*
 * bxcan_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bxcan_base.h>
#include <halm/platform/stm32/bxcan_defs.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configPins(const struct BxCanBase *,
    const struct BxCanBaseConfig *);
static bool setInstance(uint8_t, struct BxCanBase *);
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_BXCAN_NO_DEINIT
static void canDeinit(void *);
#else
#define canDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const BxCanBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = canInit,
    .deinit = canDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry bxCanPins[] = {
    {
        .key = PIN(PORT_A, 11), /* CAN_RX */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_A, 12), /* CAN_TX */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 8), /* CAN_RX */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 9), /* CAN_TX */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 0), /* CAN_RX */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_D, 1), /* CAN_TX */
        .channel = 0,
        .value = 0
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct BxCanBase *instances[2] = {0};
/*----------------------------------------------------------------------------*/
static void configPins(const struct BxCanBase *interface,
    const struct BxCanBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  pinEntry = pinFind(bxCanPins, config->rx, interface->channel);
  assert(pinEntry);

  pin = pinInit(config->rx);
  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  pinEntry = pinFind(bxCanPins, config->tx, interface->channel);
  assert(pinEntry);

  pin = pinInit(config->tx);
  pinOutput(pin, true);
  pinSetFunction(pin, pinEntry->value);
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct BxCanBase *object)
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
  /* Joint interrupt */
  if (instances[0])
    instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct BxCanBase *interface __attribute__((unused)))
{
  return clockFrequency(ApbClock);
}
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct BxCanBaseConfig * const config = configBase;
  struct BxCanBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  if (!setInstance(interface->channel, interface))
    return E_BUSY;

  /* Enable alternate functions on RX and TX pins */
  configPins(interface, config);

  sysClockEnable(CLK_CAN);
  sysResetEnable(RST_CAN);
  sysResetDisable(RST_CAN);

  interface->irq.rx0 = CEC_CAN_IRQ;
  interface->irq.rx1 = CEC_CAN_IRQ;
  interface->irq.sce = CEC_CAN_IRQ;
  interface->irq.tx = CEC_CAN_IRQ;
  interface->reg = STM_CAN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_BXCAN_NO_DEINIT
static void canDeinit(void *object)
{
  const struct BxCanBase * const interface = object;

  sysClockDisable(CLK_CAN);
  instances[interface->channel] = 0;
}
#endif
