/*
 * uart_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/lpc/uart_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  LPC_UART_Type *reg;
  /* Peripheral clock branch */
  enum SysClockBranch peripheralBranch;
  /* Clock to register interface */
  enum SysClockBranch registerBranch;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void uartDeinit(void *);
#else
#  define uartDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit
};
/*----------------------------------------------------------------------------*/
static const struct UartBlockDescriptor uartBlockEntries[] = {
    {
        .reg = (LPC_UART_Type *)LPC_USART0,
        .registerBranch = CLK_M4_USART0,
        .peripheralBranch = CLK_APB0_USART0,
        .reset = RST_USART0,
        .irq = USART0_IRQ
    }, {
        .reg = LPC_UART1,
        .registerBranch = CLK_M4_UART1,
        .peripheralBranch = CLK_APB0_UART1,
        .reset = RST_UART1,
        .irq = UART1_IRQ
    }, {
        .reg = (LPC_UART_Type *)LPC_USART2,
        .registerBranch = CLK_M4_USART2,
        .peripheralBranch = CLK_APB2_USART2,
        .reset = RST_USART2,
        .irq = USART2_IRQ
    }, {
        .reg = (LPC_UART_Type *)LPC_USART3,
        .registerBranch = CLK_M4_USART3,
        .peripheralBranch = CLK_APB2_USART3,
        .reset = RST_USART3,
        .irq = USART3_IRQ
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
static struct UartBase *instances[4] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UartBase *object)
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
void USART0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void UART1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void USART2_ISR(void)
{
  /* In M0 cores USART2 IRQ is combined with CAN1 IRQ */
  if (instances[2]->handler != NULL)
    instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void USART3_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  const void *clock = NULL;

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
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  uartConfigPins(config);

  const struct UartBlockDescriptor * const entry =
      &uartBlockEntries[config->channel];

  /* Enable clocks to register interface and peripheral */
  sysClockEnable(entry->peripheralBranch);
  sysClockEnable(entry->registerBranch);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;
  const struct UartBlockDescriptor * const entry =
      &uartBlockEntries[interface->channel];

  sysClockDisable(entry->registerBranch);
  sysClockDisable(entry->peripheralBranch);

  instances[interface->channel] = NULL;
}
#endif
