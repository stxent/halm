/*
 * uart_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/bouffalo/clocking.h>
#include <halm/platform/bouffalo/uart_base.h>
#include <halm/platform/bouffalo/uart_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_BOUFFALO_UART_NO_DEINIT
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
void uartConfigPins(const struct UartBaseConfig *config)
{
  uint32_t uartSigSel = BL_GLB->UART_SIG_SEL_0;

  if (config->rx)
  {
    /* Configure UART RX pin */
    const struct Pin pin = pinInit(config->rx);
    const unsigned int sig = pin.number % SIG_TOTAL;

    uartSigSel &= ~UART_SIG_SEL_SIG_MASK(sig);
    uartSigSel |= UART_SIG_SEL_SIG(sig, SIG_RXD(config->channel));

    pinInput(pin);
    pinSetFunction(pin, UART_FUNCTION);
  }

  if (config->tx)
  {
    /* Configure UART TX pin */
    const struct Pin pin = pinInit(config->tx);
    const unsigned int sig = pin.number % SIG_TOTAL;

    uartSigSel &= ~UART_SIG_SEL_SIG_MASK(sig);
    uartSigSel |= UART_SIG_SEL_SIG(sig, SIG_TXD(config->channel));

    pinOutput(pin, true);
    pinSetFunction(pin, UART_FUNCTION);
  }

  BL_GLB->UART_SIG_SEL_0 = uartSigSel;
}
/*----------------------------------------------------------------------------*/
static struct UartBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UartBase *object)
{
  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void UART0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void UART1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *)
{
  return clockFrequency(UartClock);
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

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = UART0_IRQ + config->channel;
  interface->reg = !config->channel ? BL_UART0 : BL_UART1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_UART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;
  instances[interface->channel] = NULL;
}
#endif
