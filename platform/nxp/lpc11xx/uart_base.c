/*
 * uart_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/system.h"
#include "platform/nxp/uart_base.h"
#include "platform/nxp/lpc11xx/interrupts.h"
#include "platform/nxp/lpc11xx/power.h"
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
static struct UartBase *descriptors[] = {0};
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass uartTable = {
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit,

    .callback = 0,
    .get = 0,
    .set = 0,
    .read = 0,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor uartPins[] = {
    {
        .key = GPIO_TO_PIN(1, 6), /* UART_RX */
        .channel = 0,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(1, 7), /* UART_TX */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *UartBase = &uartTable;
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
uint32_t uartGetClock(struct UartBase *interface __attribute__((unused)))
{
  return sysCoreClock / DEFAULT_DIV_VALUE;
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

  /* Set controller specific parameters */
  sysClockEnable(CLK_UART);
  LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV;
  interface->reg = LPC_UART;
  interface->irq = UART_IRQ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  struct UartBase *interface = object;

  /* Disable peripheral clock */
  LPC_SYSCON->UARTCLKDIV = 0;
  sysClockDisable(CLK_UART);

  /* Release interface pins */
  gpioDeinit(&interface->txPin);
  gpioDeinit(&interface->rxPin);

  /* Reset descriptor */
  setDescriptor(interface->channel, 0);
}
