/*
 * uart_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gen_1/uart_base.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  LPC_UART_Type *reg;
  /* Peripheral interrupt request identifier */
  irqNumber irq;
  /* Reset control identifier */
  enum sysDeviceReset reset;
  /* Peripheral clock branch */
  enum sysClockBranch periperalBranch;
  /* Clock to register interface */
  enum sysClockBranch registerBranch;
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct UartBase *,
    struct UartBase *);
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
        .reg = (LPC_UART_Type *)LPC_USART0,
        .irq = USART0_IRQ,
        .registerBranch = CLK_M4_USART0,
        .periperalBranch = CLK_APB0_USART0,
        .reset = RST_USART0
    },
    {
        .reg = LPC_UART1,
        .irq = UART1_IRQ,
        .registerBranch = CLK_M4_UART1,
        .periperalBranch = CLK_APB0_UART1,
        .reset = RST_UART1
    },
    {
        .reg = (LPC_UART_Type *)LPC_USART2,
        .irq = USART2_IRQ,
        .registerBranch = CLK_M4_USART2,
        .periperalBranch = CLK_APB2_USART2,
        .reset = RST_USART2
    },
    {
        .reg = (LPC_UART_Type *)LPC_USART3,
        .irq = USART3_IRQ,
        .registerBranch = CLK_M4_USART3,
        .periperalBranch = CLK_APB2_USART3,
        .reset = RST_USART3
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
    {
        .key = PIN(PORT_1, 13), /* U1_TXD */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_1, 14), /* U1_RXD */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_1, 15), /* U2_TXD */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_1, 16), /* U2_RXD */
        .channel = 2,
        .value = 1
    }, {
        .key = PIN(PORT_2, 0), /* U0_TXD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_2, 1), /* U0_RXD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_2, 3), /* U3_TXD */
        .channel = 3,
        .value = 2
    }, {
        .key = PIN(PORT_2, 4), /* U3_RXD */
        .channel = 3,
        .value = 2
    }, {
        .key = PIN(PORT_2, 10), /* U2_TXD */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(PORT_2, 11), /* U2_RXD */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(PORT_3, 4), /* U1_TXD */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_3, 5), /* U1_RXD */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_4, 1), /* U3_TXD */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_4, 2), /* U3_RXD */
        .channel = 3,
        .value = 6
    }, {
        .key = PIN(PORT_5, 6), /* U1_TXD */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_5, 7), /* U1_RXD */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_6, 4), /* U0_TXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_6, 5), /* U0_RXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_7, 1), /* U2_TXD */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_7, 2), /* U2_RXD */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_9, 3), /* U3_TXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_9, 4), /* U3_RXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_9, 5), /* U0_TXD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_9, 6), /* U0_RXD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_A, 1), /* U2_TXD */
        .channel = 2,
        .value = 3
    }, {
        .key = PIN(PORT_A, 2), /* U2_RXD */
        .channel = 2,
        .value = 3
    }, {
        .key = PIN(PORT_C, 13), /* U1_TXD */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_C, 14), /* U1_RXD */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_E, 11), /* U1_TXD */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_E, 12), /* U1_RXD */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_F, 2), /* U3_TXD */
        .channel = 3,
        .value = 1
    }, {
        .key = PIN(PORT_F, 3), /* U3_RXD */
        .channel = 3,
        .value = 1
    }, {
        .key = PIN(PORT_F, 10), /* U0_TXD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_F, 11), /* U0_RXD */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &uartTable;
static struct UartBase *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, const struct UartBase *state,
    struct UartBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void USART0_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_ISR(void)
{
  descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void USART2_ISR(void)
{
  descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void USART3_ISR(void)
{
  descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  const void *clock = 0;

  switch (interface->channel)
  {
    case 0:
      clock = Usart0Clock;
      break;

    case 1:
      clock = Uart1Clock;
      break;

    case 2:
      clock = Usart2Clock;
      break;

    case 3:
      clock = Usart3Clock;
      break;

    default:
      return 0;
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;
  enum result res;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  /* Configure input and output pins */
  uartConfigPins(interface, config);

  const struct UartBlockDescriptor *entry =
      &uartBlockEntries[interface->channel];

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->periperalBranch);
  sysClockEnable(entry->registerBranch);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;
  const struct UartBlockDescriptor *entry =
      &uartBlockEntries[interface->channel];

  sysClockDisable(entry->registerBranch);
  sysClockDisable(entry->periperalBranch);
  setDescriptor(interface->channel, interface, 0);
}
