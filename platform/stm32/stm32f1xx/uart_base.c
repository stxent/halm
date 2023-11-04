/*
 * uart_base.c
 * Copyright (C) 2016, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bdma_circular.h>
#include <halm/platform/stm32/bdma_oneshot.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/system.h>
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
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
static const struct UartBlockDescriptor *findDescriptor(uint8_t);
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
#ifdef CONFIG_PLATFORM_STM32_USART1
    {
        .reg = STM_USART1,
        .clock = CLK_USART1,
        .reset = RST_USART1,
        .irq = USART1_IRQ,
        .channel = USART1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART2
    {
        .reg = STM_USART2,
        .clock = CLK_USART2,
        .reset = RST_USART2,
        .irq = USART2_IRQ,
        .channel = USART2
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART3
    {
        .reg = STM_USART3,
        .clock = CLK_USART3,
        .reset = RST_USART3,
        .irq = USART3_IRQ,
        .channel = USART3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_UART4
    {
        .reg = STM_UART4,
        .clock = CLK_UART4,
        .reset = RST_UART4,
        .irq = UART4_IRQ,
        .channel = UART4
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_UART5
    {
        .reg = STM_UART5,
        .clock = CLK_UART5,
        .reset = RST_UART5,
        .irq = UART5_IRQ,
        .channel = UART5
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
#ifdef CONFIG_PLATFORM_STM32_USART1
    {
        .key = PIN(PORT_A, 8), /* USART1_CK */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_A, 9), /* USART1_TX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 0)
    }, {
        .key = PIN(PORT_A, 10), /* USART1_RX */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 0)
    }, {
        .key = PIN(PORT_A, 11), /* USART1_CTS */
        .channel = 0,
        .value = PACK_REMAP(REMAP_USART1, 0)
    }, {
        .key = PIN(PORT_A, 12), /* USART1_RTS */
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
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART2
    {
        .key = PIN(PORT_A, 0), /* USART2_CTS */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_A, 1), /* USART2_RTS */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_A, 2), /* USART2_TX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_A, 3), /* USART2_RX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_A, 4), /* USART2_CK */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 0)
    }, {
        .key = PIN(PORT_D, 3), /* USART2_CTS */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    }, {
        .key = PIN(PORT_D, 4), /* USART2_RTS */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    }, {
        .key = PIN(PORT_D, 5), /* USART2_TX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    }, {
        .key = PIN(PORT_D, 6), /* USART2_RX */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    }, {
        .key = PIN(PORT_D, 7), /* USART2_CK */
        .channel = 1,
        .value = PACK_REMAP(REMAP_USART2, 1)
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART3
    {
        .key = PIN(PORT_B, 10), /* USART3_TX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_B, 11), /* USART3_RX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_B, 12), /* USART3_CK */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_B, 13), /* USART3_CTS */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_B, 14), /* USART3_RTS */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 0)
    }, {
        .key = PIN(PORT_C, 10), /* USART3_TX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 1)
    }, {
        .key = PIN(PORT_C, 11), /* USART3_RX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 1)
    }, {
        .key = PIN(PORT_C, 12), /* USART3_CK */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 1)
    }, {
        .key = PIN(PORT_D, 8), /* USART3_TX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    }, {
        .key = PIN(PORT_D, 9), /* USART3_RX */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    }, {
        .key = PIN(PORT_D, 10), /* USART3_CK */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    }, {
        .key = PIN(PORT_D, 11), /* USART3_CTS */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    }, {
        .key = PIN(PORT_D, 12), /* USART3_RTS */
        .channel = 2,
        .value = PACK_REMAP(REMAP_USART3, 3)
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_UART4
    {
        .key = PIN(PORT_C, 10), /* UART4_TX */
        .channel = 3,
        .value = 0
    }, {
        .key = PIN(PORT_C, 11), /* UART4_RX */
        .channel = 3,
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_UART5
    {
        .key = PIN(PORT_C, 12), /* UART5_TX */
        .channel = 4,
        .value = 0
    }, {
        .key = PIN(PORT_D, 2), /* UART5_RX */
        .channel = 4,
        .value = 0
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instances[5] = {NULL};
/*----------------------------------------------------------------------------*/
static const struct UartBlockDescriptor *findDescriptor(uint8_t channel)
{
  for (size_t index = 0; index < ARRAY_SIZE(uartBlockEntries); ++index)
  {
    if (uartBlockEntries[index].channel == channel)
      return &uartBlockEntries[index];
  }

  return NULL;
}
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
#ifdef CONFIG_PLATFORM_STM32_USART1
void USART1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_USART2
void USART2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_USART3
void USART3_ISR(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_UART4
void UART4_ISR(void)
{
  instances[3]->handler(instances[3]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_UART5
void UART5_ISR(void)
{
  instances[4]->handler(instances[4]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  return clockFrequency(interface->channel == 0 ? Apb2Clock : Apb1Clock);
}
/*----------------------------------------------------------------------------*/
void *uartMakeCircularDma(uint8_t channel __attribute__((unused)),
    uint8_t stream, enum DmaPriority priority, enum DmaType type)
{
  const struct BdmaCircularConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(BdmaCircular, &config);
}
/*----------------------------------------------------------------------------*/
void *uartMakeOneShotDma(uint8_t channel __attribute__((unused)),
    uint8_t stream, enum DmaPriority priority, enum DmaType type)
{
  const struct BdmaOneShotConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(BdmaOneShot, &config);
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  struct UartBase * const interface = object;

  const struct UartBlockDescriptor * const entry =
      findDescriptor(config->channel);

  assert(entry != NULL);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  uartConfigPins(config);

  sysClockEnable(entry->clock);
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_UART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;
  const struct UartBlockDescriptor * const entry =
      findDescriptor(interface->channel);

  sysClockDisable(entry->clock);
  instances[interface->channel] = NULL;
}
#endif
