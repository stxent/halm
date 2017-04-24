/*
 * uart_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/uart_base.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  LPC_UART_Type *reg;
  IrqNumber irq;
  enum sysBlockPower power;
  enum sysClockBranch clock;
};
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t, const struct UartBase *, struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *, const void *);
static void uartDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass uartTable = {
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit
};
/*----------------------------------------------------------------------------*/
static const struct UartBlockDescriptor uartBlockEntries[] = {
    {
        .reg = LPC_UART0,
        .irq = UART0_IRQ,
        .power = PWR_UART0,
        .clock = CLK_UART0
    },
    {
        .reg = (LPC_UART_Type *)LPC_UART1,
        .irq = UART1_IRQ,
        .power = PWR_UART1,
        .clock = CLK_UART1
    },
    {
        .reg = LPC_UART2,
        .irq = UART2_IRQ,
        .power = PWR_UART2,
        .clock = CLK_UART2
    },
    {
        .reg = LPC_UART3,
        .irq = UART3_IRQ,
        .power = PWR_UART3,
        .clock = CLK_UART3
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
    {
        .key = PIN(0, 0), /* UART_TXD3 */
        .channel = 3,
        .value = 2
    }, {
        .key = PIN(0, 1), /* UART_RXD3 */
        .channel = 3,
        .value = 2
    }, {
        .key = PIN(0, 2), /* UART_TXD0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 3), /* UART_RXD0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 10), /* UART_TXD2 */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(0, 11), /* UART_RXD2 */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(0, 15), /* UART_TXD1 */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(0, 16), /* UART_RXD1 */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(0, 25), /* UART_TXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = PIN(0, 26), /* UART_RXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = PIN(2, 0), /* UART_TXD1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 1), /* UART_RXD1 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(2, 8), /* UART_TXD2 */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(2, 9), /* UART_RXD2 */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(4, 28), /* UART_TXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = PIN(4, 29), /* UART_RXD3 */
        .channel = 3,
        .value = 3
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &uartTable;
static struct UartBase *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t channel, const struct UartBase *state,
    struct UartBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface);
}
/*----------------------------------------------------------------------------*/
void UART0_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void UART2_ISR(void)
{
  descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void UART3_ISR(void)
{
  descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(interface->channel, 0, interface))
    return E_BUSY;

  /* Configure input and output pins */
  uartConfigPins(interface, config);

  const struct UartBlockDescriptor * const entry =
      &uartBlockEntries[interface->channel];

  sysPowerEnable(entry->power);
  sysClockControl(entry->clock, DEFAULT_DIV);

  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;

  sysPowerDisable(uartBlockEntries[interface->channel].power);
  setDescriptor(interface->channel, interface, 0);
}
