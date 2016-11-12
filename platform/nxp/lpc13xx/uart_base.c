/*
 * uart_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/uart_base.h>
#include <halm/platform/nxp/lpc13xx/clocking.h>
#include <halm/platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
/* UART clock divisor is the number from 1 to 255 or 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
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
const struct PinEntry uartPins[] = {
    {
        .key = PIN(1, 6), /* UART_RX */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(1, 7), /* UART_TX */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &uartTable;
static struct UartBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t channel, const struct UartBase *state,
    struct UartBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface);
}
/*----------------------------------------------------------------------------*/
void UART_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface __attribute__((unused)))
{
  return (clockFrequency(MainClock) * LPC_SYSCON->SYSAHBCLKDIV)
      / DEFAULT_DIV_VALUE;
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

  sysClockEnable(CLK_UART);
  LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV;
  interface->reg = LPC_UART;
  interface->irq = UART_IRQ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;

  LPC_SYSCON->UARTCLKDIV = 0; /* Disable peripheral clock */
  sysClockDisable(CLK_UART);
  setDescriptor(interface->channel, interface, 0);
}
