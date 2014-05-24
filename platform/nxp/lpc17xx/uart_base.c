/*
 * uart_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/uart_base.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct UartBase *);
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
const struct GpioDescriptor uartPins[] = {
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
const struct EntityClass *UartBase = &uartTable;
static struct UartBase *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct UartBase *interface)
{
  assert(channel < sizeof(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), 0,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void UART0_ISR(void)
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
void UART2_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void UART3_ISR(void)
{
  if (descriptors[3])
    descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(struct UartBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configPtr)
{
  const struct UartBaseConfig * const config = configPtr;
  struct UartBase *interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, interface)) != E_OK)
    return res;

  if ((res = uartSetupPins(interface, config)) != E_OK)
    return res;

  interface->handler = 0;

  switch (interface->channel)
  {
    case 0:
      sysPowerEnable(PWR_UART0);
      sysClockControl(CLK_UART0, DEFAULT_DIV);
      interface->reg = LPC_UART0;
      interface->irq = UART0_IRQ;
      break;
    case 1:
      sysPowerEnable(PWR_UART1);
      sysClockControl(CLK_UART1, DEFAULT_DIV);
      interface->reg = (LPC_UART_Type *)LPC_UART1;
      interface->irq = UART1_IRQ;
      break;
    case 2:
      sysPowerEnable(PWR_UART2);
      sysClockControl(CLK_UART2, DEFAULT_DIV);
      interface->reg = LPC_UART2;
      interface->irq = UART2_IRQ;
      break;
    case 3:
      sysPowerEnable(PWR_UART3);
      sysClockControl(CLK_UART3, DEFAULT_DIV);
      interface->reg = LPC_UART3;
      interface->irq = UART3_IRQ;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const enum sysPowerDevice uartPower[] = {
      PWR_UART0, PWR_UART1, PWR_UART2, PWR_UART3
  };
  struct UartBase *interface = object;

  sysPowerDisable(uartPower[interface->channel]);
  setDescriptor(interface->channel, 0);
}
