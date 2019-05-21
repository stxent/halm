/*
 * uart_base.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gen_1/uart_base.h>
#include <halm/platform/nxp/lpc11xx/clocking.h>
#include <halm/platform/nxp/lpc11xx/system.h>
/*----------------------------------------------------------------------------*/
/* UART clock divisor is the number from 1 to 255 or 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
/*----------------------------------------------------------------------------*/
static bool setInstance(struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_UART_NO_DEINIT
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
        .key = PIN(1, 6), /* RXD */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(1, 7), /* TXD */
        .channel = 0,
        .value = 1
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(2, 7), /* RXD */
        .channel = 0,
        .value = 2
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(3, 1), /* RXD */
        .channel = 0,
        .value = 3
    }, {
        /* Available on LPC1100XL only */
        .key = PIN(3, 4), /* RXD */
        .channel = 0,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instance = 0;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct UartBase *object)
{
  if (!instance)
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
  return (clockFrequency(MainClock) * LPC_SYSCON->SYSAHBCLKDIV)
      / DEFAULT_DIV_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  assert(config->channel == 0);

  if (!setInstance(interface))
    return E_BUSY;

  interface->reg = LPC_UART;
  interface->irq = UART_IRQ;
  interface->handler = 0;
  interface->channel = config->channel;

  /* Configure input and output pins */
  uartConfigPins(interface, config);

  sysClockEnable(CLK_UART);
  LPC_SYSCON->UARTCLKDIV = DEFAULT_DIV;

  /*
   * Configuration differs for latest silicon revisions and is not
   * completely reentrant. Device should be reset to configure the peripheral
   * with other pins.
   */
  switch (config->rx)
  {
    case PIN(2, 7):
      LPC_IOCON->RXD_LOC = 1;
      break;

    case PIN(3, 1):
      LPC_IOCON->RXD_LOC = 2;
      break;

    case PIN(3, 4):
      LPC_IOCON->RXD_LOC = 3;
      break;

    default:
      /* Do nothing to ensure compatibility with older revisions  */
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_UART_NO_DEINIT
static void uartDeinit(void *object __attribute__((unused)))
{
  /* Disable the peripheral clock */
  LPC_SYSCON->UARTCLKDIV = 0;

  sysClockDisable(CLK_UART);

  instance = 0;
}
#endif
