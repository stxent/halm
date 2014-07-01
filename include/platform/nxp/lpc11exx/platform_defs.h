/*
 * platform/nxp/lpc11exx/platform_defs.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_DEFS_H_
#define PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#define NVIC_PRIORITY_SIZE 2
/*------------------System Configuration Registers----------------------------*/
typedef struct
{
  __rw__ uint32_t SYSMEMREMAP; /* System memory remap */
  __rw__ uint32_t PRESETCTRL; /* Peripheral reset control */
  __rw__ uint32_t SYSPLLCTRL; /* System PLL control */
  __rw__ uint32_t SYSPLLSTAT;
  __ne__ uint32_t RESERVED0[4];

  /* Offset 0x20 */
  __rw__ uint32_t SYSOSCCTRL;
  __rw__ uint32_t WDTOSCCTRL;
  __rw__ uint32_t IRCCTRL;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t SYSRESSTAT; /* System reset status */
  __ne__ uint32_t RESERVED2[3];

  /* Offset 0x40 */
  __rw__ uint32_t SYSPLLCLKSEL;
  __rw__ uint32_t SYSPLLCLKUEN;
  __ne__ uint32_t RESERVED3[10];
  __rw__ uint32_t MAINCLKSEL;
  __rw__ uint32_t MAINCLKUEN;
  __rw__ uint32_t SYSAHBCLKDIV;
  __ne__ uint32_t RESERVED4;

  /* Offset 0x80 */
  __rw__ uint32_t SYSAHBCLKCTRL;
  __ne__ uint32_t RESERVED5[4];
  __rw__ uint32_t SSP0CLKDIV;
  __rw__ uint32_t UARTCLKDIV;
  __rw__ uint32_t SSP1CLKDIV;
  __ne__ uint32_t RESERVED6[16];

  /* Offset 0xE0 */
  __rw__ uint32_t CLKOUTCLKSEL;
  __rw__ uint32_t CLKOUTUEN;
  __rw__ uint32_t CLKOUTDIV;
  __ne__ uint32_t RESERVED7[5];

  /* Offset 0x100 */
  __rw__ uint32_t PIOPORCAP0;
  __rw__ uint32_t PIOPORCAP1;
  __ne__ uint32_t RESERVED10[18];

  /* Offset 0x150 */
  __rw__ uint32_t BODCTRL;
  __rw__ uint32_t SYSTCKCAL;
  __ne__ uint32_t RESERVED11[6];

  /* Offset 0x170 */
  __rw__ uint32_t IRQLATENCY;
  __rw__ uint32_t NMISRC;
  union
  {
    __rw__ uint32_t PINTSEL[8];
    struct
    {
      __rw__ uint32_t PINTSEL0;
      __rw__ uint32_t PINTSEL1;
      __rw__ uint32_t PINTSEL2;
      __rw__ uint32_t PINTSEL3;
      __rw__ uint32_t PINTSEL4;
      __rw__ uint32_t PINTSEL5;
      __rw__ uint32_t PINTSEL6;
      __rw__ uint32_t PINTSEL7;
    };
  };
  __ne__ uint32_t RESERVED12[27];

  /* Offset 0x200 */
  __ne__ uint32_t RESERVED13;
  __rw__ uint32_t STARTERP0;
  __ne__ uint32_t RESERVED14[3];
  __rw__ uint32_t STARTERP1;
  __ne__ uint32_t RESERVED15[6];

  /* Offset 0x230 */
  __rw__ uint32_t PDSLEEPCFG;
  __rw__ uint32_t PDAWAKECFG;
  __rw__ uint32_t PDRUNCFG;
  __ne__ uint32_t RESERVED16[110];
  __ro__ uint32_t DEVICE_ID;
} LPC_SYSCON_Type;
/*------------------Input/Output Configuration--------------------------------*/
typedef struct
{
  union
  {
    __rw__ uint32_t PIO0[24];
    struct
    {
      __rw__ uint32_t RESET_PIO0_0;
      __rw__ uint32_t PIO0_1;
      __rw__ uint32_t PIO0_2;
      __rw__ uint32_t PIO0_3;
      __rw__ uint32_t PIO0_4;
      __rw__ uint32_t PIO0_5;
      __rw__ uint32_t PIO0_6;
      __rw__ uint32_t PIO0_7;

      /* Offset 0x20 */
      __rw__ uint32_t PIO0_8;
      __rw__ uint32_t PIO0_9;
      __rw__ uint32_t SWCLK_PIO0_10;
      __rw__ uint32_t TDI_PIO0_11;
      __rw__ uint32_t TMS_PIO0_12;
      __rw__ uint32_t TDO_PIO0_13;
      __rw__ uint32_t TRST_PIO0_14;
      __rw__ uint32_t SWDIO_PIO0_15;

      /* Offset 0x40 */
      __rw__ uint32_t PIO0_16;
      __rw__ uint32_t PIO0_17;
      __rw__ uint32_t PIO0_18;
      __rw__ uint32_t PIO0_19;
      __rw__ uint32_t PIO0_20;
      __rw__ uint32_t PIO0_21;
      __rw__ uint32_t PIO0_22;
      __rw__ uint32_t PIO0_23;
    };
  };

  union
  {
    __rw__ uint32_t PIO1[32];
    struct
    {
      /* Offset 0x60 */
      __rw__ uint32_t PIO1_0;
      __rw__ uint32_t PIO1_1;
      __rw__ uint32_t PIO1_2;
      __rw__ uint32_t PIO1_3;
      __rw__ uint32_t PIO1_4;
      __rw__ uint32_t PIO1_5;
      __rw__ uint32_t PIO1_6;
      __rw__ uint32_t PIO1_7;

      /* Offset 0x80 */
      __rw__ uint32_t PIO1_8;
      __rw__ uint32_t PIO1_9;
      __rw__ uint32_t PIO1_10;
      __rw__ uint32_t PIO1_11;
      __rw__ uint32_t PIO1_12;
      __rw__ uint32_t PIO1_13;
      __rw__ uint32_t PIO1_14;
      __rw__ uint32_t PIO1_15;

      /* Offset 0xA0 */
      __rw__ uint32_t PIO1_16;
      __rw__ uint32_t PIO1_17;
      __rw__ uint32_t PIO1_18;
      __rw__ uint32_t PIO1_19;
      __rw__ uint32_t PIO1_20;
      __rw__ uint32_t PIO1_21;
      __rw__ uint32_t PIO1_22;
      __rw__ uint32_t PIO1_23;

      /* Offset 0xC0 */
      __rw__ uint32_t PIO1_24;
      __rw__ uint32_t PIO1_25;
      __rw__ uint32_t PIO1_26;
      __rw__ uint32_t PIO1_27;
      __rw__ uint32_t PIO1_28;
      __rw__ uint32_t PIO1_29;
      __ne__ uint32_t RESERVED0;
      __rw__ uint32_t PIO1_31;
    };
  };
} LPC_IOCON_Type;
/*------------------Power Management Unit-------------------------------------*/
typedef struct
{
  __rw__ uint32_t PCON;
  __rw__ uint32_t GPREG0;
  __rw__ uint32_t GPREG1;
  __rw__ uint32_t GPREG2;
  __rw__ uint32_t GPREG3;
  __rw__ uint32_t GPREG4;
} LPC_PMU_Type;
/*------------------General Purpose Input/Output------------------------------*/
typedef struct
{
  __rw__ uint32_t ISEL;
  __rw__ uint32_t IENR;
  __wo__ uint32_t SIENR;
  __wo__ uint32_t CIENR;
  __rw__ uint32_t IENF;
  __wo__ uint32_t SIENF;
  __wo__ uint32_t CIENF;
  __rw__ uint32_t RISE;
  __rw__ uint32_t FALL;
  __rw__ uint32_t IST;
} LPC_GPIO_INT_Type;

typedef struct
{
  __rw__ uint32_t CTRL;
  __ne__ uint32_t RESERVED0[7];

  /* Offset 0x20 */
  union
  {
    __rw__ uint32_t PORT_POL[2];
    struct
    {
      __rw__ uint32_t PORT_POL0;
      __rw__ uint32_t PORT_POL1;
    };
  };
  __ne__ uint32_t RESERVED1[6];

  /* Offset 0x40 */
  union
  {
    __rw__ uint32_t PORT_ENA[2];
    struct
    {
      __rw__ uint32_t PORT_ENA0;
      __rw__ uint32_t PORT_ENA1;
    };
  };
} LPC_GPIO_GROUP_INT_Type;

typedef struct
{
  union
  {
    __rw__ uint8_t B[64];
    struct
    {
      __rw__ uint8_t B0[32];
      __rw__ uint8_t B1[32];
    };
  };
  __ne__ uint32_t RESERVED0[1008];

  /* Offset 0x1000 */
  union
  {
    __rw__ uint32_t W[64];
    struct
    {
      __rw__ uint32_t W0[32];
      __rw__ uint32_t W1[32];
    };
  };
  __ne__ uint32_t RESERVED1[960];

  /* Offset 0x2000 */
  union
  {
    __rw__ uint32_t DIR[2];
    struct
    {
      __rw__ uint32_t DIR0;
      __rw__ uint32_t DIR1;
    };
  };
  __ne__ uint32_t RESERVED2[30];

  /* Offset 0x2080 */
  union
  {
    __rw__ uint32_t MASK[2];
    struct
    {
      __rw__ uint32_t MASK0;
      __rw__ uint32_t MASK1;
    };
  };
  __ne__ uint32_t RESERVED3[30];

  /* Offset 0x2100 */
  union
  {
    __rw__ uint32_t PIN[2];
    struct
    {
      __rw__ uint32_t PIN0;
      __rw__ uint32_t PIN1;
    };
  };
  __ne__ uint32_t RESERVED4[30];

  /* Offset 0x2180 */
  union
  {
    __rw__ uint32_t MPIN[2];
    struct
    {
      __rw__ uint32_t MPIN0;
      __rw__ uint32_t MPIN1;
    };
  };
  __ne__ uint32_t RESERVED5[30];

  /* Offset 0x2200 */
  union
  {
    __rw__ uint32_t SET[2];
    struct
    {
      __rw__ uint32_t SET0;
      __rw__ uint32_t SET1;
    };
  };
  __ne__ uint32_t RESERVED6[30];

  /* Offset 0x2280 */
  union
  {
    __wo__ uint32_t CLR[2];
    struct
    {
      __wo__ uint32_t CLR0;
      __wo__ uint32_t CLR1;
    };
  };
  __ne__ uint32_t RESERVED7[30];

  /* Offset 0x2300 */
  union
  {
    __wo__ uint32_t NOT[2];
    struct
    {
      __wo__ uint32_t NOT0;
      __wo__ uint32_t NOT1;
    };
  };
} LPC_GPIO_Type;
/*------------------Timer/Counter---------------------------------------------*/
typedef struct
{
  __rw__ uint32_t IR;
  __rw__ uint32_t TCR;
  __rw__ uint32_t TC;
  __rw__ uint32_t PR;
  __rw__ uint32_t PC;
  __rw__ uint32_t MCR;
  union
  {
    __rw__ uint32_t MR[4];
    struct
    {
      __rw__ uint32_t MR0;
      __rw__ uint32_t MR1;
      __rw__ uint32_t MR2;
      __rw__ uint32_t MR3;
    };
  };
  __rw__ uint32_t CCR;
  __ro__ uint32_t CR0;
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t CR1;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t EMR;
  __ne__ uint32_t RESERVED2[12];
  __rw__ uint32_t CTCR;
  __rw__ uint32_t PWMC;
} LPC_TIMER_Type;
/*------------------Extended Universal Asynchronous Receiver Transmitter------*/
/*
 * UART controller with modem control, RS485, half duplex mode,
 * smart card interface, synchronous mode and IrDa support.
 */
typedef struct
{
  union
  {
    __ro__ uint32_t RBR;
    __wo__ uint32_t THR;
    __rw__ uint32_t DLL;
  };
  union
  {
    __rw__ uint32_t DLM;
    __rw__ uint32_t IER;
  };
  union
  {
    __ro__ uint32_t IIR;
    __wo__ uint32_t FCR;
  };
  __rw__ uint32_t LCR;
  __rw__ uint32_t MCR;
  __ro__ uint32_t LSR;
  __ro__ uint32_t MSR;
  __rw__ uint32_t SCR;
  __rw__ uint32_t ACR;
  __rw__ uint32_t ICR;
  __rw__ uint32_t FDR;
  __rw__ uint32_t OSR;
  __rw__ uint32_t TER;
  __ne__ uint32_t RESERVED0[3];
  __rw__ uint32_t HDEN; /* Half Duplex Enable register */
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t SCICTRL; /* Smart Card Interface Control register */
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t RS485ADRMATCH;
  __rw__ uint32_t RS485DLY;
  __rw__ uint32_t SYNCCTRL; /* Synchronous Mode Control register */
} LPC_UART_Type;
/*------------------Windowed Watchdog Timer-----------------------------------*/
typedef struct
{
  __rw__ uint32_t MOD;
  __rw__ uint32_t TC;
  __wo__ uint32_t FEED;
  __ro__ uint32_t TV;
  __rw__ uint32_t CLKSEL;
  __rw__ uint32_t WARNINT;
  __rw__ uint32_t WINDOW;
} LPC_WDT_Type;
/*----------------------------------------------------------------------------*/
/* Base addresses */
#define LPC_APB_BASE                (0x40000000UL)
#define LPC_AHB_BASE                (0x50000000UL)

/* APB peripherals */
#define LPC_I2C_BASE                (LPC_APB_BASE + 0x00000)
#define LPC_WDT_BASE                (LPC_APB_BASE + 0x04000)
#define LPC_USART_BASE              (LPC_APB_BASE + 0x08000)
#define LPC_TIMER16B0_BASE          (LPC_APB_BASE + 0x0C000)
#define LPC_TIMER16B1_BASE          (LPC_APB_BASE + 0x10000)
#define LPC_TIMER32B0_BASE          (LPC_APB_BASE + 0x14000)
#define LPC_TIMER32B1_BASE          (LPC_APB_BASE + 0x18000)
#define LPC_ADC_BASE                (LPC_APB_BASE + 0x1C000)
#define LPC_PMU_BASE                (LPC_APB_BASE + 0x38000)
#define LPC_SSP0_BASE               (LPC_APB_BASE + 0x40000)
#define LPC_IOCON_BASE              (LPC_APB_BASE + 0x44000)
#define LPC_SYSCON_BASE             (LPC_APB_BASE + 0x48000)
#define LPC_GPIO_INT_BASE           (LPC_APB_BASE + 0x4C000)
#define LPC_SSP1_BASE               (LPC_APB_BASE + 0x58000)
#define LPC_GPIO_GROUP_INT0_BASE    (LPC_APB_BASE + 0x5C000)
#define LPC_GPIO_GROUP_INT1_BASE    (LPC_APB_BASE + 0x60000)

/* AHB peripherals */
#define LPC_GPIO_BASE               (LPC_AHB_BASE + 0x00000)
/*----------------------------------------------------------------------------*/
/* Peripheral declaration */
#define LPC_I2C             ((LPC_I2C_Type *)LPC_I2C_BASE)
#define LPC_WDT             ((LPC_WDT_Type *)LPC_WDT_BASE)
#define LPC_USART           ((LPC_UART_Type *)LPC_USART_BASE)
#define LPC_TIMER16B0       ((LPC_TIMER_Type *)LPC_TIMER16B0_BASE)
#define LPC_TIMER16B1       ((LPC_TIMER_Type *)LPC_TIMER16B1_BASE)
#define LPC_TIMER32B0       ((LPC_TIMER_Type *)LPC_TIMER32B0_BASE)
#define LPC_TIMER32B1       ((LPC_TIMER_Type *)LPC_TIMER32B1_BASE)
#define LPC_ADC             ((LPC_ADC_Type *)LPC_ADC_BASE)
#define LPC_PMU             ((LPC_PMU_Type *)LPC_PMU_BASE)
#define LPC_SSP0            ((LPC_SSP_Type *)LPC_SSP0_BASE)
#define LPC_SSP1            ((LPC_SSP_Type *)LPC_SSP1_BASE)
#define LPC_IOCON           ((LPC_IOCON_Type *)LPC_IOCON_BASE)
#define LPC_SYSCON          ((LPC_SYSCON_Type *)LPC_SYSCON_BASE)
#define LPC_GPIO            ((LPC_GPIO_Type *)LPC_GPIO_BASE)
#define LPC_GPIO_INT        ((LPC_GPIO_INT_Type *)LPC_GPIO_INT_BASE)
#define LPC_GPIO_GROUP_INT0 \
    ((LPC_GPIO_GROUP_INT_Type *)LPC_GPIO_GROUP_INT0_BASE)
#define LPC_GPIO_GROUP_INT1 \
    ((LPC_GPIO_GROUP_INT_Type *)LPC_GPIO_GROUP_INT1_BASE)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_DEFS_H_ */
