/*
 * lpuart_base.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/imxrt/clocking.h>
#include <halm/platform/imxrt/imxrt106x/pin_daisy.h>
#include <halm/platform/imxrt/lpuart_base.h>
#include <halm/platform/imxrt/lpuart_defs.h>
#include <halm/platform/imxrt/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct LpUartBlockDescriptor
{
  IMX_LPUART_Type *reg;
  enum SysClockBranch clock;
  enum PinDaisyIndex rxDaisy;
  enum PinDaisyIndex txDaisy;
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct LpUartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_IMXRT_LPUART_NO_DEINIT
static void uartDeinit(void *);
#else
#  define uartDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const LpUartBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = uartInit,
    .deinit = uartDeinit
};
/*----------------------------------------------------------------------------*/
static const struct LpUartBlockDescriptor lpUartBlockEntries[] = {
    {
        .reg = IMX_LPUART1,
        .clock = CLK_LPUART1,
        .rxDaisy = DAISY_UNDEFINED,
        .txDaisy = DAISY_UNDEFINED,
        .irq = LPUART1_IRQ
    }, {
        .reg = IMX_LPUART2,
        .clock = CLK_LPUART2,
        .rxDaisy = DAISY_LPUART2_RX,
        .txDaisy = DAISY_LPUART2_TX,
        .irq = LPUART2_IRQ
    }, {
        .reg = IMX_LPUART3,
        .clock = CLK_LPUART3,
        .rxDaisy = DAISY_LPUART3_RX,
        .txDaisy = DAISY_LPUART3_TX,
        .irq = LPUART3_IRQ
    }, {
        .reg = IMX_LPUART4,
        .clock = CLK_LPUART4,
        .rxDaisy = DAISY_LPUART4_RX,
        .txDaisy = DAISY_LPUART4_TX,
        .irq = LPUART4_IRQ
    }, {
        .reg = IMX_LPUART5,
        .clock = CLK_LPUART5,
        .rxDaisy = DAISY_LPUART5_RX,
        .txDaisy = DAISY_LPUART5_TX,
        .irq = LPUART5_IRQ
    }, {
        .reg = IMX_LPUART6,
        .clock = CLK_LPUART6,
        .rxDaisy = DAISY_LPUART6_RX,
        .txDaisy = DAISY_LPUART6_TX,
        .irq = LPUART6_IRQ
    }, {
        .reg = IMX_LPUART7,
        .clock = CLK_LPUART7,
        .rxDaisy = DAISY_LPUART7_RX,
        .txDaisy = DAISY_LPUART7_TX,
        .irq = LPUART7_IRQ
    }, {
        .reg = IMX_LPUART8,
        .clock = CLK_LPUART8,
        .rxDaisy = DAISY_LPUART8_RX,
        .txDaisy = DAISY_LPUART8_TX,
        .irq = LPUART8_IRQ
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry lpUartPins[] = {
    /* LPUART1 */
    {
        .key = PIN(PORT_AD_B0, 12), /* LPUART1_TXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_AD_B0, 13), /* LPUART1_RXD */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_AD_B0, 14), /* LPUART1_CTS_B */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_AD_B0, 15), /* LPUART1_RTS_B */
        .channel = 0,
        .value = 2
    },

    /* LPUART2 */
    {
        .key = PIN(PORT_SD_B1, 11), /* LPUART2_TXD */
        .channel = 1,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_AD_B1, 2), /* LPUART2_TXD */
        .channel = 1,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_AD_B1, 3), /* LPUART2_RXD */
        .channel = 1,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_SD_B1, 10), /* LPUART2_RXD */
        .channel = 1,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_AD_B1, 0), /* LPUART2_CTS_B */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_AD_B1, 1), /* LPUART2_RTS_B */
        .channel = 1,
        .value = 2
    },

    /* LPUART3 */
    {
        .key = PIN(PORT_AD_B1, 6), /* LPUART3_TXD */
        .channel = 2,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_B0, 8), /* LPUART3_TXD */
        .channel = 2,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(PORT_EMC, 13), /* LPUART3_TXD */
        .channel = 2,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_EMC, 14), /* LPUART3_RXD */
        .channel = 2,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_B0, 9), /* LPUART3_RXD */
        .channel = 2,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = PIN(PORT_AD_B1, 7), /* LPUART3_RXD */
        .channel = 2,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_EMC, 15), /* LPUART3_CTS_B */
        .channel = 2,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_AD_B1, 4), /* LPUART3_CTS_B */
        .channel = 2,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_AD_B1, 5), /* LPUART3_RTS_B */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(PORT_EMC, 16), /* LPUART3_RTS_B */
        .channel = 2,
        .value = 2
    },

    /* LPUART4 */
    {
        .key = PIN(PORT_SD_B1, 0), /* LPUART4_TXD */
        .channel = 3,
        .value = PACK_VALUE(4, 0)
    }, {
        .key = PIN(PORT_B1, 0), /* LPUART4_TXD */
        .channel = 3,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(PORT_EMC, 19), /* LPUART4_TXD */
        .channel = 3,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_SD_B1, 1), /* LPUART4_RXD */
        .channel = 3,
        .value = PACK_VALUE(4, 0)
    }, {
        .key = PIN(PORT_EMC, 20), /* LPUART4_RXD */
        .channel = 3,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_B1, 1), /* LPUART4_RXD */
        .channel = 3,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(PORT_EMC, 17), /* LPUART4_CTS_B */
        .channel = 3,
        .value = 2
    }, {
        .key = PIN(PORT_EMC, 18), /* LPUART4_RTS_B */
        .channel = 3,
        .value = 2
    },

    /* LPUART5 */
    {
        .key = PIN(PORT_EMC, 23), /* LPUART5_TXD */
        .channel = 4,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_B1, 12), /* LPUART5_TXD */
        .channel = 4,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_B1, 13), /* LPUART5_RXD */
        .channel = 4,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_EMC, 24), /* LPUART5_RXD */
        .channel = 4,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_EMC, 28), /* LPUART5_CTS_B */
        .channel = 4,
        .value = 2
    }, {
        .key = PIN(PORT_EMC, 27), /* LPUART5_RTS_B */
        .channel = 4,
        .value = 2
    },

    /* LPUART6 */
    {
        .key = PIN(PORT_EMC, 25), /* LPUART6_TXD */
        .channel = 5,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_AD_B0, 2), /* LPUART6_TXD */
        .channel = 5,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_AD_B0, 3), /* LPUART6_RXD */
        .channel = 5,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_EMC, 26), /* LPUART6_RXD */
        .channel = 5,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_EMC, 30), /* LPUART6_CTS_B */
        .channel = 5,
        .value = 2
    }, {
        .key = PIN(PORT_EMC, 29), /* LPUART6_RTS_B */
        .channel = 5,
        .value = 2
    },

    /* LPUART7 */
    {
        .key = PIN(PORT_SD_B1, 8), /* LPUART7_TXD */
        .channel = 6,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_EMC, 31), /* LPUART7_TXD */
        .channel = 6,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_EMC, 32), /* LPUART7_RXD */
        .channel = 6,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_SD_B1, 9), /* LPUART7_RXD */
        .channel = 6,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_SD_B1, 6), /* LPUART7_CTS_B */
        .channel = 6,
        .value = 2
    }, {
        .key = PIN(PORT_SD_B1, 7), /* LPUART7_RTS_B */
        .channel = 6,
        .value = 2
    },

    /* LPUART8 */
    {
        .key = PIN(PORT_EMC, 38), /* LPUART8_TXD */
        .channel = 7,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(PORT_AD_B1, 10), /* LPUART8_TXD */
        .channel = 7,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_SD_B0, 4), /* LPUART8_TXD */
        .channel = 7,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_SD_B0, 5), /* LPUART8_RXD */
        .channel = 7,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = PIN(PORT_AD_B1, 11), /* LPUART8_RXD */
        .channel = 7,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = PIN(PORT_EMC, 39), /* LPUART8_RXD */
        .channel = 7,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = PIN(PORT_SD_B0, 2), /* LPUART8_CTS_B */
        .channel = 7,
        .value = 2
    }, {
        .key = PIN(PORT_SD_B0, 3), /* LPUART8_RTS_B */
        .channel = 7,
        .value = 2
    },

    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct LpUartBase *instances[8] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct LpUartBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void LPUART1_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void LPUART2_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void LPUART3_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void LPUART4_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void LPUART5_ISR(void)
{
  instances[4]->handler(instances[4]);
}
/*----------------------------------------------------------------------------*/
void LPUART6_ISR(void)
{
  instances[5]->handler(instances[5]);
}
/*----------------------------------------------------------------------------*/
void LPUART7_ISR(void)
{
  instances[6]->handler(instances[6]);
}
/*----------------------------------------------------------------------------*/
void LPUART8_ISR(void)
{
  instances[7]->handler(instances[7]);
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct LpUartBase *)
{
  return clockFrequency(UartClock);
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct LpUartBaseConfig * const config = configBase;
  struct LpUartBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  const struct LpUartBlockDescriptor * const entry =
      &lpUartBlockEntries[config->channel];

  /* Configure input and output pins */
  uartConfigPins(config, entry->rxDaisy, entry->txDaisy);
  /* Disable clock gating */
  sysClockEnable(entry->clock);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_IMXRT_LPUART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct LpUartBase * const interface = object;

  sysClockDisable(lpUartBlockEntries[interface->channel].clock);
  instances[interface->channel] = NULL;
}
#endif
