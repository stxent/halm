/*
 * can_base.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_2/can_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct CanBlockDescriptor
{
  LPC_CAN_Type *reg;
  /* Peripheral clock and clock for register interface */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static void configPins(const struct CanBaseConfig *);
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
static const struct CanBlockDescriptor canBlockEntries[] = {
    {
        .reg = LPC_CAN0,
        .clock = CLK_APB3_CAN0,
        .reset = RST_CAN0,
        .irq = CAN0_IRQ
    }, {
        .reg = LPC_CAN1,
        .clock = CLK_APB1_CAN1,
        .reset = RST_CAN1,
        .irq = CAN1_IRQ
    }
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry canPins[] = {
    {
        .key = PIN(PORT_1, 17), /* CAN1_TD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_1, 18), /* CAN1_RD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_3, 1), /* CAN0_RD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_3, 2), /* CAN0_TD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_4, 8), /* CAN1_TD */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_4, 9), /* CAN1_RD */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_E, 0), /* CAN1_TD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 1), /* CAN1_RD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 2), /* CAN0_RD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_E, 3), /* CAN0_TD */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct CanBase *instances[2] = {0};
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
void CAN0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void CAN1_ISR(void)
{
  /* In M0 cores CAN1 IRQ is combined with USART2 IRQ */
  if (instances[1]->handler != NULL)
    instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct CanBase *interface)
{
  return clockFrequency(interface->channel == 0 ? Apb3Clock : Apb1Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct CanBaseConfig * const config = configBase;
  struct CanBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  configPins(config);

  const struct CanBlockDescriptor * const entry =
      &canBlockEntries[config->channel];

  /* Enable clock */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_CAN_NO_DEINIT
static void canDeinit(void *object)
{
  const struct CanBase * const interface = object;
  const struct CanBlockDescriptor * const entry =
      &canBlockEntries[interface->channel];

  sysClockDisable(entry->clock);
  instances[interface->channel] = NULL;
}
#endif
