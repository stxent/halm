/*
 * can_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/can_base.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
#include <limits.h>
/*----------------------------------------------------------------------------*/
struct CanBlockDescriptor
{
  NM_CAN_Type *reg;
  /* Peripheral clock and clock for register interface */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t);
static void configPins(const struct CanBaseConfig *);
static bool setInstance(uint8_t, struct CanBase *);
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_CAN_NO_DEINIT
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
#ifdef CONFIG_PLATFORM_NUMICRO_CAN0
    {
        .reg = NM_CAN0,
        .clock = CLK_CAN0,
        .reset = RST_CAN0,
        .irq = CAN0_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_CAN1
    {
        .reg = NM_CAN1,
        .clock = CLK_CAN1,
        .reset = RST_CAN1,
        .irq = CAN1_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_CAN2
    {
        .reg = NM_CAN2,
        .clock = CLK_CAN2,
        .reset = RST_CAN2,
        .irq = CAN2_IRQ
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry canPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_CAN0
    /* CAN0_RXD */
    {
        .key = PIN(PORT_A, 4), /* CAN0_RXD */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_A, 13), /* CAN0_RXD */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_B, 10), /* CAN0_RXD */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_C, 4), /* CAN0_RXD */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_D, 10), /* CAN0_RXD */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 15), /* CAN0_RXD */
        .channel = 0,
        .value = 4
    },
    /* CAN0_TXD */
    {
        .key = PIN(PORT_A, 5), /* CAN0_TXD */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_A, 12), /* CAN0_TXD */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_B, 11), /* CAN0_TXD */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_C, 5), /* CAN0_TXD */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_D, 11), /* CAN0_TXD */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 14), /* CAN0_TXD */
        .channel = 0,
        .value = 4
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_CAN1
    /* CAN1_RXD */
    {
        .key = PIN(PORT_B, 6), /* CAN1_RXD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 2), /* CAN1_RXD */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_C, 9), /* CAN1_RXD */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_D, 12), /* CAN1_RXD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 6), /* CAN1_RXD */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_F, 8), /* CAN1_RXD */
        .channel = 1,
        .value = 8
    },
    /* CAN1_TXD */
    {
        .key = PIN(PORT_B, 7), /* CAN1_TXD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_C, 3), /* CAN1_TXD */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_C, 10), /* CAN1_TXD */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_C, 13), /* CAN1_TXD */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_E, 7), /* CAN1_TXD */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_F, 9), /* CAN1_TXD */
        .channel = 1,
        .value = 8
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_CAN2
    /* CAN2_RXD */
    {
        .key = PIN(PORT_B, 8), /* CAN2_RXD */
        .channel = 2,
        .value = 12
    }, {
        .key = PIN(PORT_C, 0), /* CAN2_RXD */
        .channel = 2,
        .value = 10
    }, {
        .key = PIN(PORT_D, 8), /* CAN2_RXD */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_F, 6), /* CAN2_RXD */
        .channel = 2,
        .value = 8
    },
    /* CAN2_TXD */
    {
        .key = PIN(PORT_B, 9), /* CAN2_TXD */
        .channel = 2,
        .value = 12
    }, {
        .key = PIN(PORT_C, 1), /* CAN2_TXD */
        .channel = 2,
        .value = 10
    }, {
        .key = PIN(PORT_D, 9), /* CAN2_TXD */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_F, 7), /* CAN2_TXD */
        .channel = 2,
        .value = 8
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct CanBase *instances[3] = {0};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t channel)
{
  uint8_t index = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_CAN0
  if (channel == 0)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_CAN1
  if (channel == 1)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_CAN2
  if (channel == 2)
    return index;
  ++index;
#endif

  return UINT8_MAX;
}
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
#ifdef CONFIG_PLATFORM_NUMICRO_CAN0
void CAN0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_CAN1
void CAN1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_CAN2
void CAN2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct CanBase *interface)
{
  return clockFrequency(interface->channel == 1 ? Apb1Clock : Apb0Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct CanBaseConfig * const config = configBase;
  const size_t index = channelToIndex(config->channel);
  struct CanBase * const interface = object;

  assert(index != UINT8_MAX);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  configPins(config);

  const struct CanBlockDescriptor * const entry = &canBlockEntries[index];

  /* Enable clock */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetBlock(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_CAN_NO_DEINIT
static void canDeinit(void *object)
{
  const struct CanBase * const interface = object;
  const struct CanBlockDescriptor * const entry =
      &canBlockEntries[channelToIndex(interface->channel)];

  sysClockDisable(entry->clock);
  instances[interface->channel] = NULL;
}
#endif
