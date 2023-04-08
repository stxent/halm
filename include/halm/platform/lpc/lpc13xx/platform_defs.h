/*
 * halm/platform/lpc/lpc13xx/platform_defs.h
 * Based on original from NXP
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_PLATFORM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC13XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------System Configuration registers----------------------------*/
typedef struct
{
  __rw__ uint32_t SYSMEMREMAP; /* System memory remap */
  __rw__ uint32_t PRESETCTRL; /* Peripheral reset control */
  __rw__ uint32_t SYSPLLCTRL; /* System PLL control */
  __rw__ uint32_t SYSPLLSTAT;
  __rw__ uint32_t USBPLLCTRL; /* USB PLL control */
  __rw__ uint32_t USBPLLSTAT;
  __ne__ uint32_t RESERVED0[2];

  /* Offset 0x0020 */
  __rw__ uint32_t SYSOSCCTRL;
  __rw__ uint32_t WDTOSCCTRL;
  __rw__ uint32_t IRCCTRL;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t SYSRESSTAT; /* System reset status */
  __ne__ uint32_t RESERVED2[3];

  /* Offset 0x0040 */
  __rw__ uint32_t SYSPLLCLKSEL;
  __rw__ uint32_t SYSPLLCLKUEN;
  __rw__ uint32_t USBPLLCLKSEL;
  __rw__ uint32_t USBPLLCLKUEN;
  __ne__ uint32_t RESERVED3[8];
  __rw__ uint32_t MAINCLKSEL;
  __rw__ uint32_t MAINCLKUEN;
  __rw__ uint32_t SYSAHBCLKDIV;
  __ne__ uint32_t RESERVED4;

  /* Offset 0x0080 */
  __rw__ uint32_t SYSAHBCLKCTRL;
  __ne__ uint32_t RESERVED5[4];
  __rw__ uint32_t SSP0CLKDIV;
  __rw__ uint32_t UARTCLKDIV;
  __rw__ uint32_t SSP1CLKDIV;
  __ne__ uint32_t RESERVED6[3];
  __rw__ uint32_t TRACECLKDIV;
  __rw__ uint32_t SYSTICKCLKDIV;
  __ne__ uint32_t RESERVED7[3];

  /* Offset 0x00C0 */
  __rw__ uint32_t USBCLKSEL;
  __rw__ uint32_t USBCLKUEN;
  __rw__ uint32_t USBCLKDIV;
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t WDTCLKSEL;
  __rw__ uint32_t WDTCLKUEN;
  __rw__ uint32_t WDTCLKDIV;
  __ne__ uint32_t RESERVED9;

  /* Offset 0x00E0 */
  __rw__ uint32_t CLKOUTCLKSEL;
  __rw__ uint32_t CLKOUTUEN;
  __rw__ uint32_t CLKOUTDIV;
  __ne__ uint32_t RESERVED10[5];

  /* Offset 0x0100 */
  __rw__ uint32_t PIOPORCAP0;
  __rw__ uint32_t PIOPORCAP1;
  __ne__ uint32_t RESERVED11[18];

  /* Offset 0x0150 */
  __rw__ uint32_t BODCTRL;
  __rw__ uint32_t SYSTCKCAL;
  __ne__ uint32_t RESERVED12[42];

  /* Offset 0x0200 */
  union
  {
    struct
    {
      __rw__ uint32_t STARTAPRP0;
      __rw__ uint32_t STARTERP0;
      __wo__ uint32_t STARTRSRP0CLR;
      __ro__ uint32_t STARTSRP0;
      __rw__ uint32_t STARTAPRP1;
      __rw__ uint32_t STARTERP1;
      __wo__ uint32_t STARTRSRP1CLR;
      __ro__ uint32_t STARTSRP1;
    };
    struct
    {
      __rw__ uint32_t APRP;
      __rw__ uint32_t ERP;
      __wo__ uint32_t RSRPCLR;
      __ro__ uint32_t SRP;
    } START[2];
  };

  __ne__ uint32_t RESERVED13[4];

  /* Offset 0x0230 */
  __rw__ uint32_t PDSLEEPCFG;
  __rw__ uint32_t PDAWAKECFG;
  __rw__ uint32_t PDRUNCFG;
  __ne__ uint32_t RESERVED14[110];
  __ro__ uint32_t DEVICE_ID;
} LPC_SYSCON_Type;
/*------------------Flash Memory Configuration--------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0[4];
  __rw__ uint32_t FLASHCFG;
  __ne__ uint32_t RESERVED1[3];

  /* Offset 0x0020 */
  __rw__ uint32_t FMSSTART;
  __rw__ uint32_t FMSSTOP;
  __ne__ uint32_t RESERVED2;
  __ro__ uint32_t FMSW[4];
  __ne__ uint32_t RESERVED3[1001];

  /* Offset 0x0FE0 */
  __ro__ uint32_t FMSTAT;
  __ne__ uint32_t RESERVED4;
  __wo__ uint32_t FMSTATCLR;
} LPC_FMC_Type;
/*------------------Input/Output Configuration--------------------------------*/
typedef struct
{
  __rw__ uint32_t PIO2_6;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t PIO2_0;
  __rw__ uint32_t RESET_PIO0_0;
  __rw__ uint32_t PIO0_1;
  __rw__ uint32_t PIO1_8;
  __ne__ uint32_t RESERVED1;
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
  union
  {
    __rw__ uint32_t GPREG[4];
    struct
    {
      __rw__ uint32_t GPREG0;
      __rw__ uint32_t GPREG1;
      __rw__ uint32_t GPREG2;
      __rw__ uint32_t GPREG3;
    };
  };
  __rw__ uint32_t GPREG4;
} LPC_PMU_Type;
/*------------------General Purpose Input/Output------------------------------*/
typedef struct
{
  union
  {
    __rw__ uint32_t MASKED_ACCESS[4096];
    struct
    {
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
  __ne__ uint32_t RESERVED2[8184];
} LPC_GPIO_Type;
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t GDR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t INTEN;
  union
  {
    __ro__ uint32_t DR[8];
    struct
    {
      __ro__ uint32_t DR0;
      __ro__ uint32_t DR1;
      __ro__ uint32_t DR2;
      __ro__ uint32_t DR3;
      __ro__ uint32_t DR4;
      __ro__ uint32_t DR5;
      __ro__ uint32_t DR6;
      __ro__ uint32_t DR7;
    };
  };
  __ro__ uint32_t STAT;
} LPC_ADC_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CONSET;
  __ro__ uint32_t STAT;
  __rw__ uint32_t DAT;
  __rw__ uint32_t ADR0;
  __rw__ uint32_t SCLH;
  __rw__ uint32_t SCLL;
  __rw__ uint32_t CONCLR;
  __rw__ uint32_t MMCTRL;
  __rw__ uint32_t ADR1;
  __rw__ uint32_t ADR2;
  __rw__ uint32_t ADR3;
  __ro__ uint32_t DATA_BUFFER;
  __rw__ uint32_t MASK0;
  __rw__ uint32_t MASK1;
  __rw__ uint32_t MASK2;
  __rw__ uint32_t MASK3;
} LPC_I2C_Type;
/*------------------Synchronous Serial Port-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR0;
  __rw__ uint32_t CR1;
  __rw__ uint32_t DR;
  __ro__ uint32_t SR;
  __rw__ uint32_t CPSR;
  __rw__ uint32_t IMSC;
  __rw__ uint32_t RIS;
  __rw__ uint32_t MIS;
  __wo__ uint32_t ICR;
} LPC_SSP_Type;
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
    __ro__ uint32_t CR[4];
    struct
    {
      __ro__ uint32_t CR0;
      __ne__ uint32_t RESERVED0[3];
    };
  };
  __rw__ uint32_t EMR;
  __ne__ uint32_t RESERVED1[12];
  __rw__ uint32_t CTCR;
  __rw__ uint32_t PWMC; /* Chip-specific register */
} LPC_TIMER_Type;
/*------------------Universal Asynchronous Receiver Transmitter---------------*/
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
} LPC_WDT_Type;

typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t MOD;
      __rw__ uint32_t TC;
      __wo__ uint32_t FEED;
      __ro__ uint32_t TV;
    };

    LPC_WDT_Type BASE;
  };
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t WARNINT;
  __rw__ uint32_t WINDOW;
} LPC_WWDT_Type;
/*------------------Universal Serial Bus--------------------------------------*/
typedef struct
{
  __ro__ uint32_t USBDevIntSt; /* USB Device Interrupt Registers */
  __rw__ uint32_t USBDevIntEn;
  __wo__ uint32_t USBDevIntClr;
  __wo__ uint32_t USBDevIntSet;

  __wo__ uint32_t USBCmdCode; /* USB Device SIE Command Registers */
  __ro__ uint32_t USBCmdData;

  __ro__ uint32_t USBRxData; /* USB Device Transfer Registers */
  __wo__ uint32_t USBTxData;
  __ro__ uint32_t USBRxPLen;
  __wo__ uint32_t USBTxPLen;
  __rw__ uint32_t USBCtrl;
  __wo__ uint32_t USBDevFIQSel;
} LPC_USB_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  LPC_I2C_Type I2C;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(LPC_I2C_Type)];
  LPC_WWDT_Type WWDT;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(LPC_WWDT_Type)];
  LPC_UART_Type UART;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(LPC_UART_Type)];
  LPC_TIMER_Type CT16B0;
  __ne__ uint8_t RESERVED3[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_TIMER_Type CT16B1;
  __ne__ uint8_t RESERVED4[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_TIMER_Type CT32B0;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_TIMER_Type CT32B1;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_ADC_Type ADC;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(LPC_ADC_Type)];
  LPC_USB_Type USB;
  __ne__ uint8_t RESERVED8[0x18000 - sizeof(LPC_USB_Type)];
  LPC_PMU_Type PMU;
  __ne__ uint8_t RESERVED9[0x4000 - sizeof(LPC_PMU_Type)];
  LPC_FMC_Type FMC;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(LPC_FMC_Type)];
  LPC_SSP_Type SSP0;
  __ne__ uint8_t RESERVED11[0x4000 - sizeof(LPC_SSP_Type)];
  LPC_IOCON_Type IOCON;
  __ne__ uint8_t RESERVED12[0x4000 - sizeof(LPC_IOCON_Type)];
  LPC_SYSCON_Type SYSCON;
  __ne__ uint8_t RESERVED13[0x10000 - sizeof(LPC_SYSCON_Type)];
  LPC_SSP_Type SSP1;
} APB_DOMAIN_Type;

typedef struct
{
  LPC_GPIO_Type GPIO[4];
} AHB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern APB_DOMAIN_Type APB_DOMAIN;
extern AHB_DOMAIN_Type AHB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define LPC_I2C     (&APB_DOMAIN.I2C)
#define LPC_WDT     (&APB_DOMAIN.WWDT.BASE)
#define LPC_WWDT    (&APB_DOMAIN.WWDT)
#define LPC_UART    (&APB_DOMAIN.UART)
#define LPC_CT16B0  (&APB_DOMAIN.CT16B0)
#define LPC_CT16B1  (&APB_DOMAIN.CT16B1)
#define LPC_CT32B0  (&APB_DOMAIN.CT32B0)
#define LPC_CT32B1  (&APB_DOMAIN.CT32B1)
#define LPC_ADC     (&APB_DOMAIN.ADC)
#define LPC_USB     (&APB_DOMAIN.USB)
#define LPC_PMU     (&APB_DOMAIN.PMU)
#define LPC_FMC     (&APB_DOMAIN.FMC)
#define LPC_SSP0    (&APB_DOMAIN.SSP0)
#define LPC_IOCON   (&APB_DOMAIN.IOCON)
#define LPC_SYSCON  (&APB_DOMAIN.SYSCON)
#define LPC_SSP1    (&APB_DOMAIN.SSP1)

#define LPC_GPIO    (AHB_DOMAIN.GPIO)
#define LPC_GPIO0   (&AHB_DOMAIN.GPIO[0])
#define LPC_GPIO1   (&AHB_DOMAIN.GPIO[1])
#define LPC_GPIO2   (&AHB_DOMAIN.GPIO[2])
#define LPC_GPIO3   (&AHB_DOMAIN.GPIO[3])
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13XX_PLATFORM_DEFS_H_ */
