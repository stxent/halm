/*
 * uart_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/uart_base.h>
#include <platform/nxp/lpc11exx/clocking.h>
#include <platform/nxp/lpc11exx/system.h>
/*----------------------------------------------------------------------------*/
/* UART clock divisor is the number from 1 to 255 or 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
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
const struct PinEntry uartPins[] = {
    {
        .key = PIN(0, 18), /* UART_RX */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 19), /* UART_TX */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(1, 13), /* UART_TX */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 14), /* UART_RX */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 17), /* UART_RX */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 18), /* UART_TX */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 26), /* UART_RX */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 27), /* UART_TX */
        .channel = 0,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &uartTable;
static struct UartBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct UartBase *interface)
{
  assert(channel < sizeof(descriptors));

  if (descriptors[channel])
    return E_BUSY;

  descriptors[channel] = interface;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void UART_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / DEFAULT_DIV_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum result uartInit(void *object, const void *configPtr)
{
  const struct UartBaseConfig * const config = configPtr;
  struct UartBase * const interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, interface)) != E_OK)
    return res;

  if ((res = uartSetupPins(interface, config)) != E_OK)
    return res;

  interface->handler = 0;

  sysClockEnable(CLK_USART);
  LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV;
  interface->reg = LPC_USART;
  interface->irq = USART_IRQ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  struct UartBase * const interface = object;

  LPC_SYSCON->UARTCLKDIV = 0; /* Disable peripheral clock */
  sysClockDisable(CLK_USART);
  setDescriptor(interface->channel, 0);
}
