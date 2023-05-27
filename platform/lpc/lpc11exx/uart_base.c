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
static bool setInstance(struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void uartDeinit(void *);
#else
#define uartDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const UartBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
    {
        .key = PIN(0, 18), /* RXD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 19), /* TXD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(1, 13), /* TXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 14), /* RXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(1, 17), /* RXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 18), /* TXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 26), /* RXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(1, 27), /* TXD */
        .channel = 0,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct UartBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void UART_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface __attribute__((unused)))
{
  const uint32_t frequency = clockFrequency(MainClock);
  return frequency * LPC_SYSCON->SYSAHBCLKDIV / LPC_SYSCON->UARTCLKDIV;
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  assert(config->channel == 0);
  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  uartConfigPins(config);

  sysClockEnable(CLK_USART);
  LPC_SYSCON->UARTCLKDIV = LPC_SYSCON->SYSAHBCLKDIV;

  interface->channel = 0;
  interface->handler = NULL;
  interface->irq = USART_IRQ;
  interface->reg = LPC_USART;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void uartDeinit(void *object __attribute__((unused)))
{
  /* Disable the peripheral clock */
  LPC_SYSCON->UARTCLKDIV = 0;
  sysClockDisable(CLK_USART);

  instance = NULL;
}
#endif
