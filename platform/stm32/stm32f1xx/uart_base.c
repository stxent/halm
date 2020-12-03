/*
 * uart_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/stm32f1xx/clocking.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/stm32f1xx/system.h>
#include <halm/platform/stm32/uart_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  STM_USART_Type *reg;
  /* Peripheral clock branch */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_UART_NO_DEINIT
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
static const struct UartBlockDescriptor uartBlockEntries[] = {
    {
        .reg = STM_USART1,
        .clock = CLK_USART1,
        .reset = RST_USART1,
        .irq = USART1_IRQ
    }, {
        .reg = STM_USART2,
        .clock = CLK_USART2,
        .reset = RST_USART2,
        .irq = USART2_IRQ
    }, {
        .reg = STM_USART3,
        .clock = CLK_USART3,
        .reset = RST_USART3,
        .irq = USART3_IRQ
    }, {
        .reg = STM_UART4,
        .clock = CLK_UART4,
        .reset = RST_UART4,
        .irq = UART4_IRQ
    }, {
        .reg = STM_UART5,
        .clock = CLK_UART5,
        .reset = RST_UART5,
        .irq = UART5_IRQ
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
    {
        .key = PIN(PORT_A, 2), /* USART2_TX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_A, 3), /* USART2_RX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_A, 9), /* USART1_TX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 0)
    }, {
        .key = PIN(PORT_A, 10), /* USART1_RX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 0)
    }, {
        .key = PIN(PORT_B, 6), /* USART1_TX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 1)
    }, {
        .key = PIN(PORT_B, 7), /* USART1_RX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 1)
    }, {
        .key = PIN(PORT_B, 10), /* USART3_TX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_B, 11), /* USART3_RX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_C, 10), /* USART3_TX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 1)
    }, {
        /* Available on STM32F105 and STM32F107 series only */
        .key = PIN(PORT_C, 10), /* UART4_TX */
        .channel = 3,
        .value = 0
    }, {
        .key = PIN(PORT_C, 11), /* USART3_RX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 1)
    }, {
        /* Available on STM32F105 and STM32F107 series only */
        .key = PIN(PORT_C, 11), /* UART4_RX */
        .channel = 3,
        .value = 0
    }, {
        /* Available on STM32F105 and STM32F107 series only */
        .key = PIN(PORT_C, 12), /* UART5_TX */
        .channel = 4,
        .value = 0
    }, {
        /* Available on STM32F105 and STM32F107 series only */
        .key = PIN(PORT_D, 2), /* UART5_RX */
        .channel = 4,
        .value = 0
    }, {
        .key = PIN(PORT_D, 5), /* USART2_TX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    }, {
        .key = PIN(PORT_D, 6), /* USART2_RX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    }, {
        .key = PIN(PORT_D, 8), /* USART3_TX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    }, {
        .key = PIN(PORT_D, 9), /* USART3_RX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instances[5] = {0};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UartBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (!instances[channel])
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void USART1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void USART2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void USART3_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void UART4_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void UART5_ISR(void)
{
  instances[4]->handler(instances[4]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  return clockFrequency(interface->channel == 0 ? Apb2Clock : Apb1Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  if (!setInstance(interface->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  uartConfigPins(interface, config);

  const struct UartBlockDescriptor * const entry =
      &uartBlockEntries[interface->channel];

  sysClockEnable(entry->clock);
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_UART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;

  sysClockDisable(uartBlockEntries[interface->channel].clock);
  instances[interface->channel] = 0;
}
#endif
