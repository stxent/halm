/*
 * platform/nxp/lpc11xx/platform_defs.h
 * Based on original by NXP
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC11XX_PLATFORM_DEFS_H_
#define PLATFORM_NXP_LPC11XX_PLATFORM_DEFS_H_
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
  __ne__ uint32_t RESERVED6[4];
  __rw__ uint32_t SYSTICKCLKDIV;
  __ne__ uint32_t RESERVED7[7];
  __rw__ uint32_t WDTCLKSEL;
  __rw__ uint32_t WDTCLKUEN;
  __rw__ uint32_t WDTCLKDIV;
  __ne__ uint32_t RESERVED8;

  /* Offset 0xE0 */
  __rw__ uint32_t CLKOUTCLKSEL;
  __rw__ uint32_t CLKOUTUEN;
  __rw__ uint32_t CLKOUTDIV;
  __ne__ uint32_t RESERVED9[5];

  /* Offset 0x100 */
  __rw__ uint32_t PIOPORCAP0;
  __rw__ uint32_t PIOPORCAP1;
  __ne__ uint32_t RESERVED10[18];

  /* Offset 0x150 */
  __rw__ uint32_t BODCTRL;
  __rw__ uint32_t SYSTCKCAL;
  __ne__ uint32_t RESERVED11[6];
  __rw__ uint32_t IRQLATENCY;
  __rw__ uint32_t NMISRC;
  __ne__ uint32_t RESERVED12[34];

  /* Offset 0x200 */
  __rw__ uint32_t STARTAPRP0;
  __rw__ uint32_t STARTERP0;
  __rw__ uint32_t STARTRSRP0CLR;
  __rw__ uint32_t STARTSRP0;
  __ne__ uint32_t RESERVED13[8];

  /* Offset 0x230 */
  __rw__ uint32_t PDSLEEPCFG;
  __rw__ uint32_t PDAWAKECFG;
  __rw__ uint32_t PDRUNCFG;
  __ne__ uint32_t RESERVED14[110];
  __ro__ uint32_t DEVICE_ID;
} LPC_SYSCON_Type;
/*------------------Input/Output Configuration--------------------------------*/
typedef struct
{
  __rw__ uint32_t PIO2_6;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t PIO2_0;
  __rw__ uint32_t RESET_PIO0_0;
  __rw__ uint32_t PIO0_1;
  __rw__ uint32_t PIO1_8;
  __rw__ uint32_t SSEL1_LOC; /* Available on LPC1100XL only */
  __rw__ uint32_t PIO0_2;

  /* Offset 0x20 */
  __rw__ uint32_t PIO2_7;
  __rw__ uint32_t PIO2_8;
  __rw__ uint32_t PIO2_1;
  __rw__ uint32_t PIO0_3;
  __rw__ uint32_t PIO0_4;
  __rw__ uint32_t PIO0_5;
  __rw__ uint32_t PIO1_9;
  __rw__ uint32_t PIO3_4;

  /* Offset 0x40 */
  __rw__ uint32_t PIO2_4;
  __rw__ uint32_t PIO2_5;
  __rw__ uint32_t PIO3_5;
  __rw__ uint32_t PIO0_6;
  __rw__ uint32_t PIO0_7;
  __rw__ uint32_t PIO2_9;
  __rw__ uint32_t PIO2_10;
  __rw__ uint32_t PIO2_2;

  /* Offset 0x60 */
  __rw__ uint32_t PIO0_8;
  __rw__ uint32_t PIO0_9;
  __rw__ uint32_t SWCLK_PIO0_10;
  __rw__ uint32_t PIO1_10;
  __rw__ uint32_t PIO2_11;
  __rw__ uint32_t R_PIO0_11;
  __rw__ uint32_t R_PIO1_0;
  __rw__ uint32_t R_PIO1_1;

  /* Offset 0x80 */
  __rw__ uint32_t R_PIO1_2;
  __rw__ uint32_t PIO3_0;
  __rw__ uint32_t PIO3_1;
  __rw__ uint32_t PIO2_3;
  __rw__ uint32_t SWDIO_PIO1_3;
  __rw__ uint32_t PIO1_4;
  __rw__ uint32_t PIO1_11;
  __rw__ uint32_t PIO3_2;

  /* Offset 0xA0 */
  __rw__ uint32_t PIO1_5;
  __rw__ uint32_t PIO1_6;
  __rw__ uint32_t PIO1_7;
  __rw__ uint32_t PIO3_3;
  __rw__ uint32_t SCK_LOC;
  __rw__ uint32_t DSR_LOC;
  __rw__ uint32_t DCD_LOC;
  __rw__ uint32_t RI_LOC;

  /* Offset 0xC0: registers are available on LPC1100XL only */
  __rw__ uint32_t CT16B0_CAP0_LOC;
  __rw__ uint32_t SCK1_LOC;
  __rw__ uint32_t MISO1_LOC;
  __rw__ uint32_t MOSI1_LOC;
  __rw__ uint32_t CT32B0_CAP0_LOC;
  __rw__ uint32_t RXD_LOC;
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
  union {
    __rw__ uint32_t MASKED_ACCESS[4096];
    struct {
      __ne__ uint32_t RESERVED0[4095];
      __rw__ uint32_t DATA;
    };
  };
  __ne__ uint32_t RESERVED1[4096];
  __rw__ uint32_t DIR;
  __rw__ uint32_t IS;
  __rw__ uint32_t IBE;
  __rw__ uint32_t IEV;
  __rw__ uint32_t IE;
  __ro__ uint32_t RIS;
  __ro__ uint32_t MIS;
  __wo__ uint32_t IC;
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
  union
  {
    __ro__ uint32_t CR[2];
    struct
    {
      __ro__ uint32_t CR0;
      __ro__ uint32_t CR1; /* Unavailable on LPC1100, LPC1100C and LPC1100L */
    };
  };
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t EMR;
  __ne__ uint32_t RESERVED1[12];
  __rw__ uint32_t CTCR;
  __rw__ uint32_t PWMC; /* Chip-specific register */
} LPC_TIMER_Type;
/*------------------Extended Universal Asynchronous Receiver Transmitter------*/
/* UART controller with modem control and RS485 support */
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
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t FDR;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t TER;
  __ne__ uint32_t RESERVED2[6];
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t RS485ADRMATCH;
  __rw__ uint32_t RS485DLY;
} LPC_UART_Type;
/*------------------Windowed Watchdog Timer-----------------------------------*/
typedef struct
{
  __rw__ uint32_t MOD;
  __rw__ uint32_t TC;
  __wo__ uint32_t FEED;
  __ro__ uint32_t TV;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t WARNINT;
  __rw__ uint32_t WINDOW;
} LPC_WDT_Type;
/*------------------CAN Controller--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CNTL;
  __rw__ uint32_t STAT;
  __rw__ uint32_t EC;
  __rw__ uint32_t BT;
  __rw__ uint32_t INT;
  __rw__ uint32_t TEST;
  __rw__ uint32_t BRPE;
  __ne__ uint32_t RESERVED0;

  /* Offset 0x20 */
  __rw__ uint32_t IF1_CMDREQ;
  __rw__ uint32_t IF1_CMDMSK;
  __rw__ uint32_t IF1_MSK1;
  __rw__ uint32_t IF1_MSK2;
  __rw__ uint32_t IF1_ARB1;
  __rw__ uint32_t IF1_ARB2;
  __rw__ uint32_t IF1_MCTRL;
  __rw__ uint32_t IF1_DA1;
  __rw__ uint32_t IF1_DA2;
  __rw__ uint32_t IF1_DB1;
  __rw__ uint32_t IF1_DB2;
  __ne__ uint32_t RESERVED1[13];

  /* Offset 0x80 */
  __rw__ uint32_t IF2_CMDREQ;
  __rw__ uint32_t IF2_CMDMSK;
  __rw__ uint32_t IF2_MSK1;
  __rw__ uint32_t IF2_MSK2;
  __rw__ uint32_t IF2_ARB1;
  __rw__ uint32_t IF2_ARB2;
  __rw__ uint32_t IF2_MCTRL;
  __rw__ uint32_t IF2_DA1;
  __rw__ uint32_t IF2_DA2;
  __rw__ uint32_t IF2_DB1;
  __rw__ uint32_t IF2_DB2;
  __ne__ uint32_t RESERVED2[21];

  /* Offset 0x100 */
  __ro__ uint32_t TXREQ1;
  __ro__ uint32_t TXREQ2;
  __ne__ uint32_t RESERVED3[6];

  /* Offset 0x120 */
  __ro__ uint32_t ND1;
  __ro__ uint32_t ND2;
  __ne__ uint32_t RESERVED4[6];

  /* Offset 0x140 */
  __ro__ uint32_t IR1;
  __ro__ uint32_t IR2;
  __ne__ uint32_t RESERVED5[6];

  /* Offset 0x160 */
  __ro__ uint32_t MSGV1;
  __ro__ uint32_t MSGV2;
  __ne__ uint32_t RESERVED6[6];

  /* Offset 0x180 */
  __rw__ uint32_t CLKDIV;
} LPC_CAN_Type;
/*----------------------------------------------------------------------------*/
/* Base addresses */
#define LPC_APB_BASE            (0x40000000UL)
#define LPC_AHB_BASE            (0x50000000UL)

/* APB peripherals */
#define LPC_I2C_BASE            (LPC_APB_BASE + 0x00000)
#define LPC_WDT_BASE            (LPC_APB_BASE + 0x04000)
#define LPC_UART_BASE           (LPC_APB_BASE + 0x08000)
#define LPC_TIMER16B0_BASE      (LPC_APB_BASE + 0x0C000)
#define LPC_TIMER16B1_BASE      (LPC_APB_BASE + 0x10000)
#define LPC_TIMER32B0_BASE      (LPC_APB_BASE + 0x14000)
#define LPC_TIMER32B1_BASE      (LPC_APB_BASE + 0x18000)
#define LPC_ADC_BASE            (LPC_APB_BASE + 0x1C000)
#define LPC_PMU_BASE            (LPC_APB_BASE + 0x38000)
#define LPC_SSP0_BASE           (LPC_APB_BASE + 0x40000)
#define LPC_IOCON_BASE          (LPC_APB_BASE + 0x44000)
#define LPC_SYSCON_BASE         (LPC_APB_BASE + 0x48000)
#define LPC_CAN_BASE            (LPC_APB_BASE + 0x50000)
#define LPC_SSP1_BASE           (LPC_APB_BASE + 0x58000)

/* AHB peripherals */
#define LPC_GPIO_BASE           (LPC_AHB_BASE + 0x00000)
#define LPC_GPIO0_BASE          (LPC_GPIO_BASE + 0x00000)
#define LPC_GPIO1_BASE          (LPC_GPIO_BASE + 0x10000)
#define LPC_GPIO2_BASE          (LPC_GPIO_BASE + 0x20000)
#define LPC_GPIO3_BASE          (LPC_GPIO_BASE + 0x30000)
/*----------------------------------------------------------------------------*/
/* Peripheral declaration */
#define LPC_I2C         ((LPC_I2C_Type *)LPC_I2C_BASE)
#define LPC_WDT         ((LPC_WDT_Type *)LPC_WDT_BASE)
#define LPC_UART        ((LPC_UART_Type *)LPC_UART_BASE)
#define LPC_TIMER16B0   ((LPC_TIMER_Type *)LPC_TIMER16B0_BASE)
#define LPC_TIMER16B1   ((LPC_TIMER_Type *)LPC_TIMER16B1_BASE)
#define LPC_TIMER32B0   ((LPC_TIMER_Type *)LPC_TIMER32B0_BASE)
#define LPC_TIMER32B1   ((LPC_TIMER_Type *)LPC_TIMER32B1_BASE)
#define LPC_ADC         ((LPC_ADC_Type *)LPC_ADC_BASE)
#define LPC_PMU         ((LPC_PMU_Type *)LPC_PMU_BASE)
#define LPC_SSP0        ((LPC_SSP_Type *)LPC_SSP0_BASE)
#define LPC_SSP1        ((LPC_SSP_Type *)LPC_SSP1_BASE)
#define LPC_IOCON       ((LPC_IOCON_Type *)LPC_IOCON_BASE)
#define LPC_SYSCON      ((LPC_SYSCON_Type *)LPC_SYSCON_BASE)
#define LPC_USB         ((LPC_USB_Type *)LPC_USB_BASE)
#define LPC_GPIO0       ((LPC_GPIO_Type *)LPC_GPIO0_BASE)
#define LPC_GPIO1       ((LPC_GPIO_Type *)LPC_GPIO1_BASE)
#define LPC_GPIO2       ((LPC_GPIO_Type *)LPC_GPIO2_BASE)
#define LPC_GPIO3       ((LPC_GPIO_Type *)LPC_GPIO3_BASE)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC11XX_PLATFORM_DEFS_H_ */
