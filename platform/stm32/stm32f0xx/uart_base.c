/*
 * uart_base.c
 * Copyright (C) 2020, 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bdma_circular.h>
#include <halm/platform/stm32/bdma_oneshot.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/system.h>
#include <halm/platform/stm32/uart_base.h>
#include <halm/platform/stm32/uart_defs.h>
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
        .irq = USART3_8_IRQ,
        .channel = USART3
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART4
    {
        .reg = STM_USART4,
        .clock = CLK_USART4,
        .reset = RST_USART4,
        .irq = USART3_8_IRQ,
        .channel = USART4
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART5
    {
        .reg = STM_USART5,
        .clock = CLK_USART5,
        .reset = RST_USART5,
        .irq = USART3_8_IRQ,
        .channel = USART5
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART6
    {
        .reg = STM_USART6,
        .clock = CLK_USART6,
        .reset = RST_USART6,
        .irq = USART3_8_IRQ,
        .channel = USART6
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART7
    {
        .reg = STM_USART7,
        .clock = CLK_USART7,
        .reset = RST_USART7,
        .irq = USART3_8_IRQ,
        .channel = USART7
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART8
    {
        .reg = STM_USART8,
        .clock = CLK_USART8,
        .reset = RST_USART8,
        .irq = USART3_8_IRQ,
        .channel = USART8
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
#ifdef CONFIG_PLATFORM_STM32_USART1
    {
        .key = PIN(PORT_A, 8), /* USART1_CK */
        .channel = CHANNEL_CK(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 9), /* USART1_TX */
        .channel = CHANNEL_TX(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 10), /* USART1_RX */
        .channel = CHANNEL_RX(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 11), /* USART1_CTS */
        .channel = CHANNEL_CTS(0),
        .value = 1
    }, {
        .key = PIN(PORT_A, 12), /* USART1_RTS */
        .channel = CHANNEL_RTS(0),
        .value = 1
    }, {
        .key = PIN(PORT_B, 6), /* USART1_TX */
        .channel = CHANNEL_TX(0),
        .value = 0
    }, {
        .key = PIN(PORT_B, 7), /* USART1_RX */
        .channel = CHANNEL_RX(0),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART2
    {
        .key = PIN(PORT_A, 0), /* USART2_CTS */
        .channel = CHANNEL_CTS(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 1), /* USART2_RTS */
        .channel = CHANNEL_RTS(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 2), /* USART2_TX */
        .channel = CHANNEL_TX(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 3), /* USART2_RX */
        .channel = CHANNEL_RX(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 4), /* USART2_CK */
        .channel = CHANNEL_CK(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 14), /* USART2_TX */
        .channel = CHANNEL_TX(1),
        .value = 1
    }, {
        .key = PIN(PORT_A, 15), /* USART2_RX */
        .channel = CHANNEL_RX(1),
        .value = 1
    }, {
        .key = PIN(PORT_D, 3), /* USART2_CTS */
        .channel = CHANNEL_CTS(1),
        .value = 0
    }, {
        .key = PIN(PORT_D, 4), /* USART2_RTS */
        .channel = CHANNEL_RTS(1),
        .value = 0
    }, {
        .key = PIN(PORT_D, 5), /* USART2_TX */
        .channel = CHANNEL_TX(1),
        .value = 0
    }, {
        .key = PIN(PORT_D, 6), /* USART2_RX */
        .channel = CHANNEL_RX(1),
        .value = 0
    }, {
        .key = PIN(PORT_D, 7), /* USART2_CK */
        .channel = CHANNEL_CK(1),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART3
    {
        .key = PIN(PORT_A, 6), /* USART3_CTS */
        .channel = CHANNEL_CTS(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 0), /* USART3_CK */
        .channel = CHANNEL_CK(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 1), /* USART3_RTS */
        .channel = CHANNEL_RTS(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 10), /* USART3_TX */
        .channel = CHANNEL_TX(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 11), /* USART3_RX */
        .channel = CHANNEL_RX(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 12), /* USART3_CK */
        .channel = CHANNEL_CK(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 13), /* USART3_CTS */
        .channel = CHANNEL_CTS(2),
        .value = 4
    }, {
        .key = PIN(PORT_B, 14), /* USART3_RTS */
        .channel = CHANNEL_RTS(2),
        .value = 4
    }, {
        .key = PIN(PORT_C, 4), /* USART3_TX */
        .channel = CHANNEL_TX(2),
        .value = 1
    }, {
        .key = PIN(PORT_C, 5), /* USART3_RX */
        .channel = CHANNEL_RX(2),
        .value = 1
    }, {
        .key = PIN(PORT_C, 10), /* USART3_TX */
        .channel = CHANNEL_TX(2),
        .value = 1
    }, {
        .key = PIN(PORT_C, 11), /* USART3_RX */
        .channel = CHANNEL_RX(2),
        .value = 1
    }, {
        .key = PIN(PORT_C, 12), /* USART3_CK */
        .channel = CHANNEL_CK(2),
        .value = 1
    }, {
        .key = PIN(PORT_D, 2), /* USART3_RTS */
        .channel = CHANNEL_RTS(2),
        .value = 1
    }, {
        .key = PIN(PORT_D, 8), /* USART3_TX */
        .channel = CHANNEL_TX(2),
        .value = 0
    }, {
        .key = PIN(PORT_D, 9), /* USART3_RX */
        .channel = CHANNEL_RX(2),
        .value = 0
    }, {
        .key = PIN(PORT_D, 10), /* USART3_CK */
        .channel = CHANNEL_CK(2),
        .value = 0
    }, {
        .key = PIN(PORT_D, 11), /* USART3_CTS */
        .channel = CHANNEL_CTS(2),
        .value = 0
    }, {
        .key = PIN(PORT_D, 12), /* USART3_RTS */
        .channel = CHANNEL_RTS(2),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART4
    {
        .key = PIN(PORT_A, 0), /* USART4_TX */
        .channel = CHANNEL_TX(3),
        .value = 4
    }, {
        .key = PIN(PORT_A, 1), /* USART4_RX */
        .channel = CHANNEL_RX(3),
        .value = 4
    }, {
        .key = PIN(PORT_A, 15), /* USART4_RTS */
        .channel = CHANNEL_RTS(3),
        .value = 4
    }, {
        .key = PIN(PORT_B, 7), /* USART4_CTS */
        .channel = CHANNEL_CTS(3),
        .value = 4
    }, {
        .key = PIN(PORT_E, 8), /* USART4_TX */
        .channel = CHANNEL_TX(3),
        .value = 1
    }, {
        .key = PIN(PORT_E, 9), /* USART4_RX */
        .channel = CHANNEL_RX(3),
        .value = 1
    }, {
        .key = PIN(PORT_C, 10), /* USART4_TX */
        .channel = CHANNEL_TX(3),
        .value = 0
    }, {
        .key = PIN(PORT_C, 11), /* USART4_RX */
        .channel = CHANNEL_RX(3),
        .value = 0
    }, {
        .key = PIN(PORT_C, 12), /* USART4_CK */
        .channel = CHANNEL_CK(3),
        .value = 0
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART5
    {
        .key = PIN(PORT_B, 3), /* USART5_TX */
        .channel = CHANNEL_TX(4),
        .value = 4
    }, {
        .key = PIN(PORT_B, 4), /* USART5_RX */
        .channel = CHANNEL_RX(4),
        .value = 4
    }, {
        .key = PIN(PORT_B, 5), /* USART5_CK_RTS */
        .channel = CHANNEL_CK(4),
        .value = 4
    }, {
        .key = PIN(PORT_B, 5), /* USART5_CK_RTS */
        .channel = CHANNEL_RTS(4),
        .value = 4
    }, {
        .key = PIN(PORT_C, 12), /* USART5_TX */
        .channel = CHANNEL_TX(4),
        .value = 2
    }, {
        .key = PIN(PORT_D, 2), /* USART5_RX */
        .channel = CHANNEL_RX(4),
        .value = 2
    }, {
        .key = PIN(PORT_E, 7), /* USART5_CK_RTS */
        .channel = CHANNEL_CK(4),
        .value = 1
    }, {
        .key = PIN(PORT_E, 7), /* USART5_CK_RTS */
        .channel = CHANNEL_RTS(4),
        .value = 1
    },  {
        .key = PIN(PORT_E, 10), /* USART5_TX */
        .channel = CHANNEL_TX(4),
        .value = 1
    }, {
        .key = PIN(PORT_E, 11), /* USART5_RX */
        .channel = CHANNEL_RX(4),
        .value = 1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART6
    {
        .key = PIN(PORT_A, 4), /* USART6_TX */
        .channel = CHANNEL_TX(5),
        .value = 5
    }, {
        .key = PIN(PORT_A, 5), /* USART6_RX */
        .channel = CHANNEL_RX(5),
        .value = 5
    }, {
        .key = PIN(PORT_F, 3), /* USART6_CK_RTS */
        .channel = CHANNEL_CK(5),
        .value = 2
    }, {
        .key = PIN(PORT_C, 0), /* USART6_TX */
        .channel = CHANNEL_TX(5),
        .value = 2
    }, {
        .key = PIN(PORT_C, 1), /* USART6_RX */
        .channel = CHANNEL_RX(5),
        .value = 2
    }, {
        .key = PIN(PORT_F, 3), /* USART6_CK_RTS */
        .channel = CHANNEL_RTS(5),
        .value = 2
    }, {
        .key = PIN(PORT_F, 9), /* USART6_TX */
        .channel = CHANNEL_TX(5),
        .value = 1
    }, {
        .key = PIN(PORT_F, 10), /* USART6_RX */
        .channel = CHANNEL_RX(5),
        .value = 1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART7
    {
        .key = PIN(PORT_C, 0), /* USART7_TX */
        .channel = CHANNEL_TX(6),
        .value = 1
    }, {
        .key = PIN(PORT_C, 1), /* USART7_RX */
        .channel = CHANNEL_RX(6),
        .value = 1
    }, {
        .key = PIN(PORT_C, 6), /* USART7_TX */
        .channel = CHANNEL_TX(6),
        .value = 1
    }, {
        .key = PIN(PORT_C, 7), /* USART7_RX */
        .channel = CHANNEL_RX(6),
        .value = 1
    }, {
        .key = PIN(PORT_D, 15), /* USART7_CK_RTS */
        .channel = CHANNEL_CK(6),
        .value = 2
    }, {
        .key = PIN(PORT_D, 15), /* USART7_CK_RTS */
        .channel = CHANNEL_RTS(6),
        .value = 2
    }, {
        .key = PIN(PORT_F, 2), /* USART7_TX */
        .channel = CHANNEL_TX(6),
        .value = 1
    }, {
        .key = PIN(PORT_F, 2), /* USART7_CK_RTS */
        .channel = CHANNEL_CK(6),
        .value = 2
    }, {
        .key = PIN(PORT_F, 2), /* USART7_CK_RTS */
        .channel = CHANNEL_RTS(6),
        .value = 2
    }, {
        .key = PIN(PORT_F, 3), /* USART7_RX */
        .channel = CHANNEL_RX(6),
        .value = 1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_USART8
    {
        .key = PIN(PORT_C, 2), /* USART8_TX */
        .channel = CHANNEL_TX(7),
        .value = 2
    }, {
        .key = PIN(PORT_C, 3), /* USART8_RX */
        .channel = CHANNEL_RX(7),
        .value = 2
    }, {
        .key = PIN(PORT_C, 8), /* USART8_TX */
        .channel = CHANNEL_TX(7),
        .value = 1
    }, {
        .key = PIN(PORT_C, 9), /* USART8_RX */
        .channel = CHANNEL_RX(7),
        .value = 1
    }, {
        .key = PIN(PORT_D, 12), /* USART8_CK_RTS */
        .channel = CHANNEL_CK(7),
        .value = 2
    }, {
        .key = PIN(PORT_D, 12), /* USART8_CK_RTS */
        .channel = CHANNEL_RTS(7),
        .value = 2
    }, {
        .key = PIN(PORT_D, 13), /* USART8_TX */
        .channel = CHANNEL_TX(7),
        .value = 0
    }, {
        .key = PIN(PORT_D, 14), /* USART8_RX */
        .channel = CHANNEL_RX(7),
        .value = 0
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instances[8] = {NULL};
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
#if defined(CONFIG_PLATFORM_STM32_USART3) \
    || defined(CONFIG_PLATFORM_STM32_USART4) \
    || defined(CONFIG_PLATFORM_STM32_USART5) \
    || defined(CONFIG_PLATFORM_STM32_USART6) \
    || defined(CONFIG_PLATFORM_STM32_USART7) \
    || defined(CONFIG_PLATFORM_STM32_USART8)
void USART3_8_ISR(void)
{
  for (size_t index = 2; index <= 7; ++index)
  {
    if (instances[index] != NULL)
      instances[index]->handler(instances[index]);
  }
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  const void *clock;

  switch (interface->channel)
  {
#ifdef CONFIG_PLATFORM_STM32_USART1
    case USART1:
      clock = Usart1Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_STM32_USART2
    case USART2:
      clock = Usart2Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_STM32_USART3
    case USART3:
      clock = Usart3Clock;
      break;
#endif

    default:
      clock = ApbClock;
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
void *uartMakeCircularDma(uint8_t channel __attribute__((unused)),
    uint8_t stream, enum DmaPriority priority, enum DmaType type)
{
  const struct BdmaCircularConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = type,
      .stream = stream,
      .silent = false
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
