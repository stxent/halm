/*
 * uart_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/uart_base.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  LPC_UART_Type *reg;
  volatile uint32_t *registerClock;
  volatile uint32_t *peripheralClock;
  irq_t irq;
  enum sysDeviceReset reset;
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
static const struct UartBlockDescriptor uartBlockEntries[4] = {
    {
        .reg = (LPC_UART_Type *)LPC_USART0,
        .registerClock = &LPC_CCU1->CLK_M4_USART0_CFG,
        .peripheralClock = &LPC_CCU2->CLK_APB0_USART0_CFG,
        .irq = USART0_IRQ,
        .reset = RST_USART0
    },
    {
        .reg = LPC_UART1,
        .registerClock = &LPC_CCU1->CLK_M4_UART1_CFG,
        .peripheralClock = &LPC_CCU2->CLK_APB0_UART1_CFG,
        .irq = UART1_IRQ,
        .reset = RST_UART1
    },
    {
        .reg = (LPC_UART_Type *)LPC_USART2,
        .registerClock = &LPC_CCU1->CLK_M4_USART2_CFG,
        .peripheralClock = &LPC_CCU2->CLK_APB2_USART3_CFG,
        .irq = USART2_IRQ,
        .reset = RST_USART2
    },
    {
        .reg = (LPC_UART_Type *)LPC_USART3,
        .registerClock = &LPC_CCU1->CLK_M4_USART3_CFG,
        .peripheralClock = &LPC_CCU2->CLK_APB2_USART3_CFG,
        .irq = USART3_IRQ,
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
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void USART2_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void USART3_ISR(void)
{
  if (descriptors[3])
    descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface __attribute__((unused)))
{
  //TODO
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configPtr)
{
  const struct UartBaseConfig * const config = configPtr;
  struct UartBase * const interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  if ((res = uartSetupPins(interface, config)) != E_OK)
    return res;

  const struct UartBlockDescriptor entry = uartBlockEntries[interface->channel];

  /* Enable clocks to registers and peripheral */
  //TODO
  /* Clear registers to default values */
  sysResetEnable(entry.reset);

  interface->handler = 0;
  interface->irq = entry.irq;
  interface->reg = entry.reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;

  //TODO Disable clocks
  setDescriptor(interface->channel, interface, 0);
}
