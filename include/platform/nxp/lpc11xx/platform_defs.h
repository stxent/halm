/*
 * platform/nxp/lpc11xx/platform_defs.h
 * Based on original by NXP
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
         uint32_t RESERVED0[4];

  /* Offset 0x20 */
  __rw__ uint32_t SYSOSCCTRL;
  __rw__ uint32_t WDTOSCCTRL;
  __rw__ uint32_t IRCCTRL;
         uint32_t RESERVED1[1];
  __rw__ uint32_t SYSRESSTAT; /* System reset status */
         uint32_t RESERVED2[3];

  /* Offset 0x40 */
  __rw__ uint32_t SYSPLLCLKSEL;
  __rw__ uint32_t SYSPLLCLKUEN;
         uint32_t RESERVED3[10];
  __rw__ uint32_t MAINCLKSEL;
  __rw__ uint32_t MAINCLKUEN;
  __rw__ uint32_t SYSAHBCLKDIV;
         uint32_t RESERVED4[1];

  /* Offset 0x80 */
  __rw__ uint32_t SYSAHBCLKCTRL;
         uint32_t RESERVED5[4];
  __rw__ uint32_t SSP0CLKDIV;
  __rw__ uint32_t UARTCLKDIV;
  __rw__ uint32_t SSP1CLKDIV;
         uint32_t RESERVED6[4];
  __rw__ uint32_t SYSTICKCLKDIV;
         uint32_t RESERVED7[7];
  __rw__ uint32_t WDTCLKSEL;
  __rw__ uint32_t WDTCLKUEN;
  __rw__ uint32_t WDTCLKDIV;
         uint32_t RESERVED8[1];

  /* Offset 0xE0 */
  __rw__ uint32_t CLKOUTCLKSEL;
  __rw__ uint32_t CLKOUTUEN;
  __rw__ uint32_t CLKOUTDIV;
         uint32_t RESERVED9[5];

  /* Offset 0x100 */
  __rw__ uint32_t PIOPORCAP0;
  __rw__ uint32_t PIOPORCAP1;
         uint32_t RESERVED10[18];

  /* Offset 0x150 */
  __rw__ uint32_t BODCTRL;
         uint32_t RESERVED11[1];
  __rw__ uint32_t SYSTCKCAL;
         uint32_t RESERVED12[1];
  __rw__ uint32_t MAINREGVOUT0CFG;
  __rw__ uint32_t MAINREGVOUT1CFG;
         uint32_t RESERVED13[38];

  /* Offset 0x200 */
  __rw__ uint32_t STARTAPRP0;
  __rw__ uint32_t STARTERP0;
  __rw__ uint32_t STARTRSRP0CLR;
  __rw__ uint32_t STARTSRP0;
         uint32_t RESERVED14[8];

  /* Offset 0x230 */
  __rw__ uint32_t PDSLEEPCFG;
  __rw__ uint32_t PDAWAKECFG;
  __rw__ uint32_t PDRUNCFG;
         uint32_t RESERVED15[101];
  __w__  uint32_t VOUTCFGPROT;
         uint32_t RESERVED16[8];
  __r__  uint32_t DEVICE_ID;
} LPC_SYSCON_Type;
/*------------------Input/Output Configuration--------------------------------*/
typedef struct
{
  __rw__ uint32_t PIO2_6;
         uint32_t RESERVED0[1];
  __rw__ uint32_t PIO2_0;
  __rw__ uint32_t RESET_PIO0_0;
  __rw__ uint32_t PIO0_1;
  __rw__ uint32_t PIO1_8;
         uint32_t RESERVED1[1];
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
             uint32_t RESERVED0[4095];
      __rw__ uint32_t DATA;
    };
  };
         uint32_t RESERVED1[4096];
  __rw__ uint32_t DIR;
  __rw__ uint32_t IS;
  __rw__ uint32_t IBE;
  __rw__ uint32_t IEV;
  __rw__ uint32_t IE;
  __r__  uint32_t RIS;
  __r__  uint32_t MIS;
  __w__  uint32_t IC;
} LPC_GPIO_Type;
/*------------------Watchdog Timer--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t MOD;
  __rw__ uint32_t TC;
  __w__  uint32_t FEED;
  __r__  uint32_t TV;
         uint32_t RESERVED0;
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
         uint32_t RESERVED0;

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
         uint32_t RESERVED1[13];

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
         uint32_t RESERVED2[21];

  /* Offset 0x100 */
  __r__  uint32_t TXREQ1;
  __r__  uint32_t TXREQ2;
         uint32_t RESERVED3[6];

  /* Offset 0x120 */
  __r__  uint32_t ND1;
  __r__  uint32_t ND2;
         uint32_t RESERVED4[6];

  /* Offset 0x140 */
  __r__  uint32_t IR1;
  __r__  uint32_t IR2;
         uint32_t RESERVED5[6];

  /* Offset 0x160 */
  __r__  uint32_t MSGV1;
  __r__  uint32_t MSGV2;
         uint32_t RESERVED6[6];

  /* Offset 0x180 */
  __rw__ uint32_t CLKDIV;
} LPC_CAN_Type;
/*----------------------------------------------------------------------------*/
/* Base addresses */
#define LPC_FLASH_BASE          (0x00000000UL)
#define LPC_RAM_BASE            (0x10000000UL)
#define LPC_APB0_BASE           (0x40000000UL)
#define LPC_AHB_BASE            (0x50000000UL)

/* APB0 peripherals */
#define LPC_I2C_BASE            (LPC_APB0_BASE + 0x00000)
#define LPC_WDT_BASE            (LPC_APB0_BASE + 0x04000)
#define LPC_UART_BASE           (LPC_APB0_BASE + 0x08000)
#define LPC_TIMER16B0_BASE      (LPC_APB0_BASE + 0x0C000)
#define LPC_TIMER16B1_BASE      (LPC_APB0_BASE + 0x10000)
#define LPC_TIMER32B0_BASE      (LPC_APB0_BASE + 0x14000)
#define LPC_TIMER32B1_BASE      (LPC_APB0_BASE + 0x18000)
#define LPC_ADC_BASE            (LPC_APB0_BASE + 0x1C000)
#define LPC_PMU_BASE            (LPC_APB0_BASE + 0x38000)
#define LPC_SSP0_BASE           (LPC_APB0_BASE + 0x40000)
#define LPC_IOCON_BASE          (LPC_APB0_BASE + 0x44000)
#define LPC_SYSCON_BASE         (LPC_APB0_BASE + 0x48000)
#define LPC_CAN_BASE            (LPC_APB0_BASE + 0x50000)
#define LPC_SSP1_BASE           (LPC_APB0_BASE + 0x58000)

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
#define LPC_UART        ((LPC_UART_MODEM_Type *)LPC_UART_BASE)
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
#endif /* PLATFORM_DEFS_H_ */
