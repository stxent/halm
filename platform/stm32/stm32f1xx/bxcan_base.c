/*
 * bxcan_base.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bxcan_base.h>
#include <halm/platform/stm32/bxcan_defs.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct BxCanBlockDescriptor
{
  STM_CAN_Type *reg;
  /* Peripheral clock branch */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Interrupt request identifiers */
  struct
  {
    IrqNumber rx0;
    IrqNumber rx1;
    IrqNumber sce;
    IrqNumber tx;
  } irq;
};
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
static const struct BxCanBlockDescriptor bxCanBlockEntries[] = {
    {
        .reg = STM_CAN1,
        .clock = CLK_CAN1,
        .reset = RST_CAN1,
        .irq = {
            .rx0 = CAN1_RX0_IRQ,
            .rx1 = CAN1_RX1_IRQ,
            .sce = CAN1_SCE_IRQ,
            .tx = CAN1_TX_IRQ,
        }
    }, {
        .reg = STM_CAN2,
        .clock = CLK_CAN2,
        .reset = RST_CAN2,
        .irq = {
            .rx0 = CAN2_RX0_IRQ,
            .rx1 = CAN2_RX1_IRQ,
            .sce = CAN2_SCE_IRQ,
            .tx = CAN2_TX_IRQ,
        }
    }
};
/*----------------------------------------------------------------------------*/
static const struct PinEntry bxCanPins[] = {
    {
        .key = PIN(PORT_A, 11), /* CAN1_RX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_CAN1, 0)
    }, {
        .key = PIN(PORT_A, 12), /* CAN1_TX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_CAN1, 0)
    }, {
        .key = PIN(PORT_B, 5), /* CAN2_RX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_CAN2, 1)
    }, {
        .key = PIN(PORT_B, 6), /* CAN2_TX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_CAN2, 1)
    }, {
        .key = PIN(PORT_B, 8), /* CAN1_RX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_CAN1, 2)
    }, {
        .key = PIN(PORT_B, 9), /* CAN1_TX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_CAN1, 2)
    }, {
        .key = PIN(PORT_B, 12), /* CAN2_RX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_CAN2, 0)
    }, {
        .key = PIN(PORT_B, 13), /* CAN2_TX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_CAN2, 0)
    }, {
        .key = PIN(PORT_D, 0), /* CAN1_RX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_CAN1, 3)
    }, {
        .key = PIN(PORT_D, 1), /* CAN1_TX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_CAN1, 3)
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
void CAN1_TX_ISR(void)
{
  /* Joint interrupt */
  if (instances[0])
    instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void CAN1_RX0_ISR(void)
{
  /* Joint interrupt */
  if (instances[0])
    instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void CAN1_RX1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void CAN1_SCE_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void CAN2_TX_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void CAN2_RX0_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void CAN2_RX1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void CAN2_SCE_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct BxCanBase *interface __attribute__((unused)))
{
  return clockFrequency(Apb1Clock);
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

  const struct BxCanBlockDescriptor * const entry =
      &bxCanBlockEntries[interface->channel];

  sysClockEnable(entry->clock);
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  interface->irq.rx0 = entry->irq.rx0;
  interface->irq.rx1 = entry->irq.rx1;
  interface->irq.sce = entry->irq.sce;
  interface->irq.tx = entry->irq.tx;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_BXCAN_NO_DEINIT
static void canDeinit(void *object)
{
  const struct BxCanBase * const interface = object;

  sysClockDisable(bxCanBlockEntries[interface->channel].clock);
  instances[interface->channel] = 0;
}
#endif
