/*
 * can_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/can_base.h>
#include <halm/platform/nxp/gen_1/can_defs.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static void configPins(const struct CanBase *, const struct CanBaseConfig *);
static bool setDescriptor(uint8_t, const struct CanBase *state,
    struct CanBase *);
/*----------------------------------------------------------------------------*/
static enum result canInit(void *, const void *);
static void canDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass canTable = {
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
const struct EntityClass * const CanBase = &canTable;
static struct CanBase *descriptors[2] = {0};
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
static bool setDescriptor(uint8_t channel, const struct CanBase *state,
    struct CanBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface);
}
/*----------------------------------------------------------------------------*/
void CAN_ISR(void)
{
  static const uint32_t mask = ICR_RI | ICR_TI_MASK;

  if (descriptors[0] && (LPC_CAN1->ICR & mask))
    descriptors[0]->handler(descriptors[0]);

  if (descriptors[1] && (LPC_CAN2->ICR & mask))
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t canGetClock(const struct CanBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result canInit(void *object, const void *configBase)
{
  const struct CanBaseConfig * const config = configBase;
  struct CanBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(interface->channel, 0, interface))
    return E_BUSY;

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

  if (!descriptors[!interface->channel])
    irqEnable(CAN_IRQ);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void canDeinit(void *object)
{
  const struct CanBase * const interface = object;

  if (!descriptors[!interface->channel])
    irqDisable(CAN_IRQ);

  switch (interface->channel)
  {
    case 0:
      sysPowerDisable(PWR_CAN1);
      break;

    case 1:
      sysPowerDisable(PWR_CAN2);
      break;
  }

  setDescriptor(interface->channel, interface, 0);
}
