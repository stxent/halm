/*
 * uart_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/numicro/uart_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct UartBlockDescriptor
{
  NM_UART_Type *reg;
  /* Clock branch identifier */
  enum SysClockBranch branch;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
  /* FIFO depth */
  uint8_t depth;
};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t);
static void disableInterrupts(IrqNumber);
static bool setInstance(uint8_t, struct UartBase *);
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_UART_NO_DEINIT
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
#ifdef CONFIG_PLATFORM_NUMICRO_UART0
    {
        .reg = NM_UART0,
        .branch = CLK_UART0,
        .reset = RST_UART0,
        .irq = UART02_IRQ,
        .depth = UART_DEPTH_16
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART1
    {
        .reg = NM_UART1,
        .branch = CLK_UART1,
        .reset = RST_UART1,
        .irq = UART13_IRQ,
        .depth = UART_DEPTH_16
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART2
    {
        .reg = NM_UART2,
        .branch = CLK_UART2,
        .reset = RST_UART2,
        .irq = UART02_IRQ,
        .depth = UART_DEPTH_1
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART3
    {
        .reg = NM_UART3,
        .branch = CLK_UART3,
        .reset = RST_UART3,
        .irq = UART13_IRQ,
        .depth = UART_DEPTH_1
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART4
    {
        .reg = NM_UART4,
        .branch = CLK_UART4,
        .reset = RST_UART4,
        .irq = UART46_IRQ,
        .depth = UART_DEPTH_16
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART5
    {
        .reg = NM_UART5,
        .branch = CLK_UART5,
        .reset = RST_UART5,
        .irq = UART57_IRQ,
        .depth = UART_DEPTH_16
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART6
    {
        .reg = NM_UART6,
        .branch = CLK_UART6,
        .reset = RST_UART6,
        .irq = UART46_IRQ,
        .depth = UART_DEPTH_1
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART7
    {
        .reg = NM_UART7,
        .branch = CLK_UART7,
        .reset = RST_UART7,
        .irq = UART57_IRQ,
        .depth = UART_DEPTH_1
    },
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry uartPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_UART0
    /* UART0_RXD */
    {
        .key = PIN(PORT_A, 0), /* UART0_RXD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_A, 4), /* UART0_RXD */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_A, 6), /* UART0_RXD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_A, 15), /* UART0_RXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_B, 8), /* UART0_RXD */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 12), /* UART0_RXD */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_C, 11), /* UART0_RXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_D, 2), /* UART0_RXD */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_F, 1), /* UART0_RXD */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 2), /* UART0_RXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_H, 11), /* UART0_RXD */
        .channel = 0,
        .value = 8
    },
    /* UART0_TXD */
    {
        .key = PIN(PORT_A, 1), /* UART0_TXD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_A, 5), /* UART0_TXD */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_A, 7), /* UART0_TXD */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_A, 14), /* UART0_TXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_B, 9), /* UART0_TXD */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 13), /* UART0_TXD */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_C, 12), /* UART0_TXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_D, 3), /* UART0_TXD */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_F, 0), /* UART0_TXD */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 3), /* UART0_TXD */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_H, 10), /* UART0_TXD */
        .channel = 0,
        .value = 8
    },
    /* UART0_nCTS */
    {
        .key = PIN(PORT_A, 5), /* UART0_nCTS */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_B, 11), /* UART0_nCTS */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 15), /* UART0_nCTS */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_C, 7), /* UART0_nCTS */
        .channel = 0,
        .value = 7
    },
    /* UART0_nRTS */
    {
        .key = PIN(PORT_A, 4), /* UART0_nRTS */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_B, 10), /* UART0_nRTS */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 14), /* UART0_nRTS */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_C, 6), /* UART0_nRTS */
        .channel = 0,
        .value = 7
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART1
    /* UART1_RXD */
    {
        .key = PIN(PORT_A, 2), /* UART1_RXD */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_A, 8), /* UART1_RXD */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_B, 2), /* UART1_RXD */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_B, 6), /* UART1_RXD */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_C, 8), /* UART1_RXD */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_D, 6), /* UART1_RXD */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_D, 10), /* UART1_RXD */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_F, 1), /* UART1_RXD */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_H, 9), /* UART1_RXD */
        .channel = 1,
        .value = 10
    },
    /* UART1_TXD */
    {
        .key = PIN(PORT_A, 3), /* UART1_TXD */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_A, 9), /* UART1_TXD */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_B, 3), /* UART1_TXD */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_B, 7), /* UART1_TXD */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_D, 7), /* UART1_TXD */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_D, 11), /* UART1_TXD */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_E, 13), /* UART1_TXD */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_F, 0), /* UART1_TXD */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_H, 8), /* UART1_TXD */
        .channel = 1,
        .value = 10
    },
    /* UART1_nCTS */
    {
        .key = PIN(PORT_A, 1), /* UART1_nCTS */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_B, 9), /* UART1_nCTS */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_E, 11), /* UART1_nCTS */
        .channel = 1,
        .value = 8
    },
    /* UART1_nRTS */
    {
        .key = PIN(PORT_A, 0), /* UART1_nRTS */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_B, 8), /* UART1_nRTS */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_E, 12), /* UART1_nRTS */
        .channel = 1,
        .value = 8
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART2
    /* UART2_RXD */
    {
        .key = PIN(PORT_B, 0), /* UART2_RXD */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_B, 4), /* UART2_RXD */
        .channel = 2,
        .value = 13
    }, {
        .key = PIN(PORT_C, 0), /* UART2_RXD */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_C, 4), /* UART2_RXD */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_D, 12), /* UART2_RXD */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_E, 9), /* UART2_RXD */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_E, 15), /* UART2_RXD */
        .channel = 2,
        .value = 3
    }, {
        .key = PIN(PORT_F, 5), /* UART2_RXD */
        .channel = 2,
        .value = 2
    },
    /* UART2_TXD */
    {
        .key = PIN(PORT_B, 1), /* UART2_TXD */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_B, 5), /* UART2_TXD */
        .channel = 2,
        .value = 13
    }, {
        .key = PIN(PORT_C, 1), /* UART2_TXD */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_C, 5), /* UART2_TXD */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_C, 13), /* UART2_TXD */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_E, 8), /* UART2_TXD */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_E, 14), /* UART2_TXD */
        .channel = 2,
        .value = 3
    }, {
        .key = PIN(PORT_F, 4), /* UART2_TXD */
        .channel = 2,
        .value = 2
    },
    /* UART2_nCTS */
    {
        .key = PIN(PORT_C, 2), /* UART2_nCTS */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_D, 9), /* UART2_nCTS */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_F, 5), /* UART2_nCTS */
        .channel = 2,
        .value = 4
    },
    /* UART2_nRTS */
    {
        .key = PIN(PORT_C, 3), /* UART2_nRTS */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_D, 8), /* UART2_nRTS */
        .channel = 2,
        .value = 4
    }, {
        .key = PIN(PORT_F, 4), /* UART2_nRTS */
        .channel = 2,
        .value = 4
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART3
    /* UART3_RXD */
    {
        .key = PIN(PORT_B, 14), /* UART3_RXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_C, 2), /* UART3_RXD */
        .channel = 3,
        .value = 11
    }, {
        .key = PIN(PORT_C, 9), /* UART3_RXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_D, 0), /* UART3_RXD */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_E, 0), /* UART3_RXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_E, 11), /* UART3_RXD */
        .channel = 3,
        .value = 7
    },
    /* UART3_TXD */
    {
        .key = PIN(PORT_B, 15), /* UART3_TXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_C, 3), /* UART3_TXD */
        .channel = 3,
        .value = 11
    }, {
        .key = PIN(PORT_C, 10), /* UART3_TXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_D, 1), /* UART3_TXD */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_E, 1), /* UART3_TXD */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_E, 10), /* UART3_TXD */
        .channel = 3,
        .value = 7
    },
    /* UART3_nCTS */
    {
        .key = PIN(PORT_B, 12), /* UART3_nCTS */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_D, 2), /* UART3_nCTS */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_H, 9), /* UART3_nCTS */
        .channel = 3,
        .value = 7
    },
    /* UART3_nRTS */
    {
        .key = PIN(PORT_B, 13), /* UART3_nRTS */
        .channel = 3,
        .value = 7
    }, {
        .key = PIN(PORT_D, 3), /* UART3_nRTS */
        .channel = 3,
        .value = 5
    }, {
        .key = PIN(PORT_H, 8), /* UART3_nRTS */
        .channel = 3,
        .value = 7
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART4
    /* UART4_RXD */
    {
        .key = PIN(PORT_A, 2), /* UART4_RXD */
        .channel = 4,
        .value = 5
    }, {
        .key = PIN(PORT_A, 13), /* UART4_RXD */
        .channel = 4,
        .value = 3
    }, {
        .key = PIN(PORT_B, 10), /* UART4_RXD */
        .channel = 4,
        .value = 6
    }, {
        .key = PIN(PORT_C, 4), /* UART4_RXD */
        .channel = 4,
        .value = 11
    }, {
        .key = PIN(PORT_C, 6), /* UART4_RXD */
        .channel = 4,
        .value = 5
    }, {
        .key = PIN(PORT_F, 6), /* UART4_RXD */
        .channel = 4,
        .value = 6
    }, {
        .key = PIN(PORT_H, 11), /* UART4_RXD */
        .channel = 4,
        .value = 7
    },
    /* UART4_TXD */
    {
        .key = PIN(PORT_A, 3), /* UART4_TXD */
        .channel = 4,
        .value = 5
    }, {
        .key = PIN(PORT_A, 12), /* UART4_TXD */
        .channel = 4,
        .value = 3
    }, {
        .key = PIN(PORT_B, 11), /* UART4_TXD */
        .channel = 4,
        .value = 6
    }, {
        .key = PIN(PORT_C, 5), /* UART4_TXD */
        .channel = 4,
        .value = 11
    }, {
        .key = PIN(PORT_C, 7), /* UART4_TXD */
        .channel = 4,
        .value = 5
    }, {
        .key = PIN(PORT_F, 7), /* UART4_TXD */
        .channel = 4,
        .value = 6
    }, {
        .key = PIN(PORT_H, 10), /* UART4_TXD */
        .channel = 4,
        .value = 7
    },
    /* UART4_nCTS */
    {
        .key = PIN(PORT_C, 8), /* UART4_nCTS */
        .channel = 4,
        .value = 5
    }, {
        .key = PIN(PORT_E, 1), /* UART4_nCTS */
        .channel = 4,
        .value = 9
    },
    /* UART4_nRTS */
    {
        .key = PIN(PORT_E, 0), /* UART4_nRTS */
        .channel = 4,
        .value = 9
    }, {
        .key = PIN(PORT_E, 13), /* UART4_nRTS */
        .channel = 4,
        .value = 5
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART5
    /* UART5_RXD */
    {
        .key = PIN(PORT_A, 4), /* UART5_RXD */
        .channel = 5,
        .value = 11
    }, {
        .key = PIN(PORT_B, 4), /* UART5_RXD */
        .channel = 5,
        .value = 7
    }, {
        .key = PIN(PORT_E, 6), /* UART5_RXD */
        .channel = 5,
        .value = 8
    }, {
        .key = PIN(PORT_F, 10), /* UART5_RXD */
        .channel = 5,
        .value = 6
    },
    /* UART5_TXD */
    {
        .key = PIN(PORT_A, 5), /* UART5_TXD */
        .channel = 5,
        .value = 11
    }, {
        .key = PIN(PORT_B, 5), /* UART5_TXD */
        .channel = 5,
        .value = 7
    }, {
        .key = PIN(PORT_E, 7), /* UART5_TXD */
        .channel = 5,
        .value = 8
    }, {
        .key = PIN(PORT_F, 11), /* UART5_TXD */
        .channel = 5,
        .value = 6
    },
    /* UART5_nCTS */
    {
        .key = PIN(PORT_B, 2), /* UART5_nCTS */
        .channel = 5,
        .value = 7
    }, {
        .key = PIN(PORT_F, 8), /* UART5_nCTS */
        .channel = 5,
        .value = 6
    },
    /* UART5_nRTS */
    {
        .key = PIN(PORT_B, 3), /* UART5_nRTS */
        .channel = 5,
        .value = 7
    }, {
        .key = PIN(PORT_F, 9), /* UART5_nRTS */
        .channel = 5,
        .value = 6
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART6
    /* UART6_RXD */
    {
        .key = PIN(PORT_A, 10), /* UART6_RXD */
        .channel = 6,
        .value = 8
    }, {
        .key = PIN(PORT_C, 6), /* UART6_RXD */
        .channel = 6,
        .value = 9
    }, {
        .key = PIN(PORT_C, 11), /* UART6_RXD */
        .channel = 6,
        .value = 5
    }, {
        .key = PIN(PORT_E, 4), /* UART6_RXD */
        .channel = 6,
        .value = 8
    }, {
        .key = PIN(PORT_E, 15), /* UART6_RXD */
        .channel = 6,
        .value = 6
    }, {
        .key = PIN(PORT_G, 14), /* UART6_RXD */
        .channel = 6,
        .value = 6
    }, {
        .key = PIN(PORT_H, 5), /* UART6_RXD */
        .channel = 6,
        .value = 5
    },
    /* UART6_TXD */
    {
        .key = PIN(PORT_A, 11), /* UART6_TXD */
        .channel = 6,
        .value = 8
    }, {
        .key = PIN(PORT_C, 7), /* UART6_TXD */
        .channel = 6,
        .value = 9
    }, {
        .key = PIN(PORT_C, 12), /* UART6_TXD */
        .channel = 6,
        .value = 5
    }, {
        .key = PIN(PORT_E, 5), /* UART6_TXD */
        .channel = 6,
        .value = 8
    }, {
        .key = PIN(PORT_E, 14), /* UART6_TXD */
        .channel = 6,
        .value = 6
    }, {
        .key = PIN(PORT_G, 13), /* UART6_TXD */
        .channel = 6,
        .value = 6
    }, {
        .key = PIN(PORT_H, 4), /* UART6_TXD */
        .channel = 6,
        .value = 5
    },
    /* UART6_nCTS */
    {
        .key = PIN(PORT_C, 9), /* UART6_nCTS */
        .channel = 6,
        .value = 5
    }, {
        .key = PIN(PORT_E, 2), /* UART6_nCTS */
        .channel = 6,
        .value = 8
    },
    /* UART6_nRTS */
    {
        .key = PIN(PORT_C, 10), /* UART6_nRTS */
        .channel = 6,
        .value = 5
    }, {
        .key = PIN(PORT_E, 3), /* UART6_nRTS */
        .channel = 6,
        .value = 8
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART7
    /* UART7_RXD */
    {
        .key = PIN(PORT_A, 8), /* UART7_RXD */
        .channel = 7,
        .value = 8
    }, {
        .key = PIN(PORT_B, 8), /* UART7_RXD */
        .channel = 7,
        .value = 8
    }, {
        .key = PIN(PORT_D, 8), /* UART7_RXD */
        .channel = 7,
        .value = 5
    }, {
        .key = PIN(PORT_E, 2), /* UART7_RXD */
        .channel = 7,
        .value = 9
    }, {
        .key = PIN(PORT_G, 12), /* UART7_RXD */
        .channel = 7,
        .value = 6
    }, {
        .key = PIN(PORT_H, 7), /* UART7_RXD */
        .channel = 7,
        .value = 4
    },
    /* UART7_TXD */
    {
        .key = PIN(PORT_A, 9), /* UART7_TXD */
        .channel = 7,
        .value = 8
    }, {
        .key = PIN(PORT_B, 9), /* UART7_TXD */
        .channel = 7,
        .value = 8
    }, {
        .key = PIN(PORT_D, 9), /* UART7_TXD */
        .channel = 7,
        .value = 5
    }, {
        .key = PIN(PORT_E, 3), /* UART7_TXD */
        .channel = 7,
        .value = 9
    }, {
        .key = PIN(PORT_G, 11), /* UART7_TXD */
        .channel = 7,
        .value = 6
    }, {
        .key = PIN(PORT_H, 6), /* UART7_TXD */
        .channel = 7,
        .value = 4
    },
    /* UART7_nCTS */
    {
        .key = PIN(PORT_E, 4), /* UART7_nCTS */
        .channel = 7,
        .value = 9
    }, {
        .key = PIN(PORT_H, 5), /* UART7_nCTS */
        .channel = 7,
        .value = 4
    },
    /* UART7_nRTS */
    {
        .key = PIN(PORT_E, 5), /* UART7_nRTS */
        .channel = 7,
        .value = 9
    }, {
        .key = PIN(PORT_H, 4), /* UART7_nRTS */
        .channel = 7,
        .value = 4
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct UartBase *instances[8] = {NULL};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t channel)
{
  uint8_t index = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_UART0
  if (channel == 0)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART1
  if (channel == 1)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART2
  if (channel == 2)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART3
  if (channel == 3)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART4
  if (channel == 4)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART5
  if (channel == 5)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART6
  if (channel == 6)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_UART7
  if (channel == 7)
    return index;
  ++index;
#endif

  return UINT8_MAX;
}
/*----------------------------------------------------------------------------*/
static void disableInterrupts(IrqNumber irq)
{
  switch (irq)
  {
    case UART02_IRQ:
      if (!(instances[0] && instances[2]))
        irqDisable(irq);
      break;

    case UART13_IRQ:
      if (!(instances[1] && instances[3]))
        irqDisable(irq);
      break;

    case UART46_IRQ:
      if (!(instances[4] && instances[6]))
        irqDisable(irq);
      break;

    case UART57_IRQ:
      if (!(instances[5] && instances[7]))
        irqDisable(irq);
      break;

    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct UartBase *object)
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
void UART02_ISR(void)
{
#ifdef CONFIG_PLATFORM_NUMICRO_UART0
  if (instances[0] != NULL)
    instances[0]->handler(instances[0]);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART2
  if (instances[2] != NULL)
    instances[2]->handler(instances[2]);
#endif
}
/*----------------------------------------------------------------------------*/
void UART13_ISR(void)
{
#ifdef CONFIG_PLATFORM_NUMICRO_UART1
  if (instances[1] != NULL)
    instances[1]->handler(instances[1]);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART3
  if (instances[3] != NULL)
    instances[3]->handler(instances[3]);
#endif
}
/*----------------------------------------------------------------------------*/
void UART46_ISR(void)
{
#ifdef CONFIG_PLATFORM_NUMICRO_UART4
  if (instances[4] != NULL)
    instances[4]->handler(instances[4]);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART6
  if (instances[6] != NULL)
    instances[6]->handler(instances[6]);
#endif
}
/*----------------------------------------------------------------------------*/
void UART57_ISR(void)
{
#ifdef CONFIG_PLATFORM_NUMICRO_UART5
  if (instances[5] != NULL)
    instances[5]->handler(instances[5]);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART7
  if (instances[7] != NULL)
    instances[7]->handler(instances[7]);
#endif
}
/*----------------------------------------------------------------------------*/
uint32_t uartGetClock(const struct UartBase *interface)
{
  const void *clock = NULL;

  switch (interface->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_UART0
    case 0:
      clock = Uart0Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART1
    case 1:
      clock = Uart1Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART2
    case 2:
      clock = Uart2Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART3
    case 3:
      clock = Uart3Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART4
    case 4:
      clock = Uart4Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART5
    case 5:
      clock = Uart5Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART6
    case 6:
      clock = Uart6Clock;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_UART7
    case 7:
      clock = Uart7Clock;
      break;
#endif
  }

  return clockFrequency(clock);
}
/*----------------------------------------------------------------------------*/
static enum Result uartInit(void *object, const void *configBase)
{
  const struct UartBaseConfig * const config = configBase;
  const size_t index = channelToIndex(config->channel);
  struct UartBase * const interface = object;

  assert(index != UINT8_MAX);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  /* Configure input and output pins */
  uartConfigPins(config);

  const struct UartBlockDescriptor * const entry = &uartBlockEntries[index];

  /* Enable clock to peripheral */
  sysClockEnable(entry->branch);
  /* Reset registers to default values */
  sysResetBlock(entry->reset);

  interface->channel = config->channel;
  interface->depth = entry->depth;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  if (!irqStatus(interface->irq))
  {
    irqSetPriority(interface->irq, config->priority);
    irqEnable(interface->irq);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_UART_NO_DEINIT
static void uartDeinit(void *object)
{
  const struct UartBase * const interface = object;
  const struct UartBlockDescriptor * const entry =
      &uartBlockEntries[channelToIndex(interface->channel)];

  disableInterrupts(interface->irq);
  sysClockDisable(entry->branch);
  instances[interface->channel] = NULL;
}
#endif
