/*
 * uart_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <halm/platform/stm/stm32f1xx/clocking.h>
#include <halm/platform/stm/stm32f1xx/system.h>
#include <halm/platform/stm/uart_base.h>
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  STM_USART_Type *reg;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
  /* Reset control identifier */
  enum sysBlockReset reset;
  /* Peripheral clock branch */
  enum sysClockBranch branch;
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
        .reg = STM_USART1,
        .irq = USART1_IRQ,
        .branch = CLK_USART1,
        .reset = RST_USART1
    },
    {
        .reg = STM_USART2,
        .irq = USART2_IRQ,
        .branch = CLK_USART2,
        .reset = RST_USART2
    },
    {
        .reg = STM_USART3,
        .irq = USART3_IRQ,
        .branch = CLK_USART3,
        .reset = RST_USART3
    },
    {
        .reg = STM_UART4,
        .irq = UART4_IRQ,
        .branch = CLK_UART4,
        .reset = RST_UART4
    },
    {
        .reg = STM_UART5,
        .irq = UART5_IRQ,
        .branch = CLK_UART5,
        .reset = RST_UART5
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
    {
        .key = PIN(PORT_A, 2), /* USART2_TX */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_A, 3), /* USART2_RX */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_B, 10), /* USART3_TX */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_B, 11), /* USART3_RX */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_D, 8), /* USART3_TX */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_D, 9), /* USART3_RX */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_A, 9), /* USART1_TX */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_A, 10), /* USART1_RX */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_C, 10), /* USART3_TX */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_C, 11), /* USART3_RX */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_D, 5), /* USART2_TX */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_D, 6), /* USART2_RX */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_B, 6), /* USART1_TX */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_B, 7), /* USART1_RX */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &uartTable;
static struct UartBase *descriptors[5] = {0};
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t channel, const struct UartBase *state,
    struct UartBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface);
}
/*----------------------------------------------------------------------------*/
void USART1_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void USART2_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void USART3_ISR(void)
{
  descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void UART4_ISR(void)
{
  descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
void UART5_ISR(void)
{
  descriptors[4]->handler(descriptors[4]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  assert(interface->channel <= 4);

  const void * const clock = !interface->channel ? Apb2Clock : Apb1Clock;

  return clockFrequency(clock);
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

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->branch);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;

  sysClockDisable(uartBlockEntries[interface->channel].branch);
  setDescriptor(interface->channel, interface, 0);
}
