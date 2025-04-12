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
static void configPins(const struct BxCanBaseConfig *);
static bool setInstance(struct BxCanBase *);
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_BXCAN_NO_DEINIT
static void canDeinit(void *);
#else
#  define canDeinit deletedDestructorTrap
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
static struct BxCanBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configPins(const struct BxCanBaseConfig *config)
{
  const struct PinEntry *pinEntry;
  struct Pin pin;

  if (config->rx)
  {
    pinEntry = pinFind(bxCanPins, config->rx, config->channel);
    assert(pinEntry != NULL);
    pinInput((pin = pinInit(config->rx)));
    pinSetFunction(pin, pinEntry->value);
  }

  if (config->tx)
  {
    pinEntry = pinFind(bxCanPins, config->tx, config->channel);
    assert(pinEntry != NULL);
    pinOutput((pin = pinInit(config->tx)), true);
    pinSetFunction(pin, pinEntry->value);
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(struct BxCanBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void CAN_ISR(void)
{
  /* Joint interrupt */
  if (instance != NULL)
    instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct BxCanBase *)
{
  return clockFrequency(ApbClock);
}
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct BxCanBaseConfig * const config = configBase;
  struct BxCanBase * const interface = object;

  assert(config->channel == 0);
  if (!setInstance(interface))
    return E_BUSY;

  /* Enable alternate functions on RX and TX pins */
  configPins(config);

  sysClockEnable(CLK_CAN);
  sysResetPulse(RST_CAN);

  interface->channel = 0;
  interface->handler = NULL;
  interface->irq.rx0 = CEC_CAN_IRQ;
  interface->irq.rx1 = CEC_CAN_IRQ;
  interface->irq.sce = CEC_CAN_IRQ;
  interface->irq.tx = CEC_CAN_IRQ;
  interface->reg = STM_CAN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_BXCAN_NO_DEINIT
static void canDeinit(void *)
{
  sysClockDisable(CLK_CAN);
  instance = NULL;
}
#endif
