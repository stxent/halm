/*
 * halm/platform/lpc/lpc82x/platform_defs.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_PLATFORM_DEFS_H_
#define HALM_PLATFORM_LPC_LPC82X_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------Analog Comparator-----------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* Comparator control register */
  __rw__ uint32_t LAD; /* Voltage Ladder register */
} LPC_ACMP_Type;
/*------------------Analog-to-Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL; /* Control register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t SEQ_CTRL[2]; /* Sequence A & B Control register */
  __rw__ uint32_t SEQ_GDAT[2]; /* Sequence A & B Global Data register */
  __ne__ uint32_t RESERVED1[2];
  __ro__ uint32_t DAT[12]; /* Channel Data Registers */
  __rw__ uint32_t THR_LOW[2]; /* Low Compare Threshold registers */
  __rw__ uint32_t THR_HIGH[2]; /* High Compare Threshold registers */
  __rw__ uint32_t CHAN_THRSEL; /* Channel Threshold Select register */
  __rw__ uint32_t INTEN; /* Interrupt Enable register */
  __rw__ uint32_t FLAGS; /* Flags register */
  __rw__ uint32_t TRM; /* Trim register */
} LPC_ADC_Type;
/*------------------CRC calculation unit--------------------------------------*/
typedef struct
{
  __rw__ uint32_t MODE; /* Mode register */
  __rw__ uint32_t SEED; /* Seed register */

  union
  {
    __ro__ uint32_t SUM; /* Checksum register */
    __wo__ uint32_t WRDATA32; /* 32-bit Write Data register */
    __wo__ uint16_t WRDATA16; /* 16-bit Write Data register */
    __wo__ uint8_t WRDATA8; /* 8-bit Write Data register */
  };
} LPC_CRC_Type;
/*------------------DMA controller--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CFG; /* Channel Configuration register */
  __ro__ uint32_t CTLSTAT; /* Channel Control and Status register */
  __rw__ uint32_t XFERCFG; /* Channel Transfer Configuration register */
  __ne__ uint32_t RESERVED0;
} LPC_SDMA_CHANNEL_Type;

typedef struct
{
  /* Offset 0x000 */
  __rw__ uint32_t CTRL; /* DMA control register */
  __ro__ uint32_t INTSTAT; /* DMA Interrupt Status register */
  __rw__ uint32_t SRAMBASE; /* DMA SRAM address of the configuration table */
  __ne__ uint32_t RESERVED0[5];

  /* Offset 0x020 */
  __rw__ uint32_t ENABLESET; /* Channel Enable read and Set */
  __ne__ uint32_t RESERVED1;
  __wo__ uint32_t ENABLECLR; /* Channel Enable Clear */
  __ne__ uint32_t RESERVED2;
  __ro__ uint32_t ACTIVE; /* Channel Active status */
  __ne__ uint32_t RESERVED3;
  __ro__ uint32_t BUSY; /* Channel Busy status */
  __ne__ uint32_t RESERVED4;
  __rw__ uint32_t ERRINT; /* Error Interrupt status */
  __ne__ uint32_t RESERVED5;
  __rw__ uint32_t INTENSET; /* Interrupt Enable read and Set */
  __ne__ uint32_t RESERVED6;
  __wo__ uint32_t INTENCLR; /* Interrupt Enable Clear */
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t INTA; /* Interrupt A status */
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t INTB; /* Interrupt B status */
  __ne__ uint32_t RESERVED9;
  __wo__ uint32_t SETVALID; /* Set Valid Pending control bits */
  __ne__ uint32_t RESERVED10;
  __wo__ uint32_t SETTRIG; /* Set Trigger control bits */
  __ne__ uint32_t RESERVED11;
  __wo__ uint32_t ABORT; /* Channel Abort control */
  __ne__ uint32_t RESERVED12[225];

  /* Offset 0x400 */
  LPC_SDMA_CHANNEL_Type CHANNELS[18];
} LPC_SDMA_Type;
/*------------------DMA trigger pin muxing block------------------------------*/
typedef struct
{
  __rw__ uint32_t DMA_ITRIG_INMUX[18];
} LPC_DMA_INMUX_Type;
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
  __ro__ uint32_t FMSW[1];
} LPC_FMC_Type;
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
  __rw__ uint32_t PMCTRL;
  __rw__ uint32_t PMSRC;
  __rw__ uint32_t PMCFG;
} LPC_GPIO_INT_Type;

typedef struct
{
  __rw__ uint8_t B[32];
  __ne__ uint32_t RESERVED0[1016];

  /* Offset 0x1000 */
  __rw__ uint32_t W[32];
  __ne__ uint32_t RESERVED1[992];

  /* Offset 0x2000 */
  __rw__ uint32_t DIR[1];
  __ne__ uint32_t RESERVED2[31];

  /* Offset 0x2080 */
  __rw__ uint32_t MASK[1];
  __ne__ uint32_t RESERVED3[31];

  /* Offset 0x2100 */
  __rw__ uint32_t PIN[1];
  __ne__ uint32_t RESERVED4[31];

  /* Offset 0x2180 */
  __rw__ uint32_t MPIN[1];
  __ne__ uint32_t RESERVED5[31];

  /* Offset 0x2200 */
  __rw__ uint32_t SET[1];
  __ne__ uint32_t RESERVED6[31];

  /* Offset 0x2280 */
  __wo__ uint32_t CLR[1];
  __ne__ uint32_t RESERVED7[31];

  /* Offset 0x2300 */
  __wo__ uint32_t NOT[1];
  __ne__ uint32_t RESERVED8[31];

  /* Offset 0x2380 */
  __wo__ uint32_t DIRSET[1];
  __ne__ uint32_t RESERVED9[31];

  /* Offset 0x2400 */
  __wo__ uint32_t DIRCLR[1];
  __ne__ uint32_t RESERVED10[31];

  /* Offset 0x2480 */
  __wo__ uint32_t DIRNOT[1];
} LPC_GPIO_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CFG; /* Configuration register */
  __rw__ uint32_t STAT; /* Status register */
  __rw__ uint32_t INTENSET; /* Interrupt Enable Set register */
  __wo__ uint32_t INTENCLR; /* Interrupt Enable Clear register */
  __rw__ uint32_t TIMEOUT; /* Timeout value register */
  __rw__ uint32_t CLKDIV; /* Clock Divider register */
  __ro__ uint32_t INTSTAT; /* Interrupt Status register */
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t MSTCTL; /* Master Control register */
  __rw__ uint32_t MSTTIME; /* Master Time register for SCL */
  __rw__ uint32_t MSTDAT; /* Master Data register */
  __ne__ uint32_t RESERVED1[5];
  __rw__ uint32_t SLVCTL; /* Slave Control register */
  __rw__ uint32_t SLVDAT; /* Slave Data register */
  __rw__ uint32_t SLVADR[4]; /* Slave Address registers */
  __rw__ uint32_t SLVQUAL0; /* Slave address Qualifier 0 register */
  __ne__ uint32_t RESERVED2[9];
  __ro__ uint32_t MONRXDAT; /* I2C Monitor Data register */
} LPC_I2C_Type;
/*------------------Input muxing block----------------------------------------*/
typedef struct
{
  __rw__ uint32_t DMA_INMUX_INMUX[2]; /* DMA Trigger Input 20 and 21 */
  __ne__ uint32_t RESERVED[6];
  __rw__ uint32_t SCT0_INMUX[4]; /* Input mux registers for SCT0 */
} LPC_INMUX_Type;
/*------------------Input/Output Configuration--------------------------------*/
typedef struct
{
  union
  {
    __rw__ uint32_t PIO0[32];

    struct
    {
      __rw__ uint32_t PIO0_17;
      __rw__ uint32_t PIO0_13;
      __rw__ uint32_t PIO0_12;
      __rw__ uint32_t PIO0_5;
      __rw__ uint32_t PIO0_4;
      __rw__ uint32_t PIO0_3;
      __rw__ uint32_t PIO0_2;
      __rw__ uint32_t PIO0_11;

      /* Offset 0x20 */
      __rw__ uint32_t PIO0_10;
      __rw__ uint32_t PIO0_16;
      __rw__ uint32_t PIO0_15;
      __rw__ uint32_t PIO0_1;
      __ne__ uint32_t RESERVED0;
      __rw__ uint32_t PIO0_9;
      __rw__ uint32_t PIO0_8;
      __rw__ uint32_t PIO0_7;

      /* Offset 0x40 */
      __rw__ uint32_t PIO0_6;
      __rw__ uint32_t PIO0_0;
      __rw__ uint32_t PIO0_14;
      __rw__ uint32_t RESERVED1;
      __rw__ uint32_t PIO0_28;
      __rw__ uint32_t PIO0_27;
      __rw__ uint32_t PIO0_26;
      __rw__ uint32_t PIO0_25;

      /* Offset 0x60 */
      __rw__ uint32_t PIO0_24;
      __rw__ uint32_t PIO0_23;
      __rw__ uint32_t PIO0_22;
      __rw__ uint32_t PIO0_21;
      __rw__ uint32_t PIO0_20;
      __rw__ uint32_t PIO0_19;
      __rw__ uint32_t PIO0_18;
      __rw__ uint32_t RESERVED2;
    };
  };
} LPC_IOCON_Type;
/*------------------Multi Rate Timer------------------------------------------*/
typedef struct
{
  __rw__ uint32_t INTVAL; /* Timer Interval register */
  __wo__ uint32_t TIMER; /* Timer register */
  __rw__ uint32_t CTRL; /* Timer Control register */
  __rw__ uint32_t STAT; /* Timer Status register */
} LPC_MRT_CHANNEL_Type;

typedef struct
{
  LPC_MRT_CHANNEL_Type CHANNEL[4];
  __ne__ uint32_t RESERVED0[45];
  __wo__ uint32_t IDLE_CH;
  __rw__ uint32_t IRQ_FLAG;
} LPC_MRT_Type;
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
  __rw__ uint32_t DPDCTRL;
} LPC_PMU_Type;
/*------------------Serial Peripheral Interface-------------------------------*/
typedef struct
{
  __rw__ uint32_t CFG; /* Configuration register */
  __rw__ uint32_t DLY; /* Delay register */
  __rw__ uint32_t STAT; /* Status register */
  __rw__ uint32_t INTENSET; /* Interrupt Enable Set register */
  __wo__ uint32_t INTENCLR; /* Interrupt Enable Clear register */
  __ro__ uint32_t RXDAT; /* Receive Data register */
  __rw__ uint32_t TXDATCTL; /* Transmit Data with Control register */
  __rw__ uint32_t TXDAT; /* Transmit Data register */
  __rw__ uint32_t TXCTL; /* Transmit Control register */
  __rw__ uint32_t DIV; /* Clock Divider register */
  __ro__ uint32_t INTSTAT; /* Interrupt Status register */
} LPC_SPI_Type;
/*------------------Switch Matrix---------------------------------------------*/
typedef struct
{
  __rw__ uint32_t PINASSIGN[12];
  __ne__ uint32_t RESERVED0[100];
  __rw__ uint32_t PINENABLE[1];
} LPC_SWM_Type;
/*------------------State Configurable Timer----------------------------------*/
typedef struct
{
  __rw__ uint32_t CONFIG;
  union
  {
    __rw__ uint32_t CTRL;
    __rw__ uint16_t CTRL_PART[2];
    struct
    {
      __rw__ uint16_t CTRL_L;
      __rw__ uint16_t CTRL_H;
    };
  };
  union
  {
    __rw__ uint32_t LIMIT;
    __rw__ uint16_t LIMIT_PART[2];
    struct
    {
      __rw__ uint16_t LIMIT_L;
      __rw__ uint16_t LIMIT_H;
    };
  };
  union
  {
    __rw__ uint32_t HALT;
    __rw__ uint16_t HALT_PART[2];
    struct
    {
      __rw__ uint16_t HALT_L;
      __rw__ uint16_t HALT_H;
    };
  };
  union
  {
    __rw__ uint32_t STOP;
    __rw__ uint16_t STOP_PART[2];
    struct
    {
      __rw__ uint16_t STOP_L;
      __rw__ uint16_t STOP_H;
    };
  };
  union
  {
    __rw__ uint32_t START;
    __rw__ uint16_t START_PART[2];
    struct
    {
      __rw__ uint16_t START_L;
      __rw__ uint16_t START_H;
    };
  };
  union
  {
    __rw__ uint32_t DITHER;
    __rw__ uint16_t DITHER_PART[2];
    struct
    {
      __rw__ uint16_t DITHER_L;
      __rw__ uint16_t DITHER_H;
    };
  };
  __ne__ uint32_t RESERVED0[9];

  /* Offset 0x0040 */
  union
  {
    __rw__ uint32_t COUNT;
    __rw__ uint16_t COUNT_PART[2];
    struct
    {
      __rw__ uint16_t COUNT_L;
      __rw__ uint16_t COUNT_H;
    };
  };
  union
  {
    __rw__ uint32_t STATE;
    __rw__ uint16_t STATE_PART[2];
    struct
    {
      __rw__ uint16_t STATE_L;
      __rw__ uint16_t STATE_H;
    };
  };
  __ro__ uint32_t INPUT;
  union
  {
    __rw__ uint32_t REGMODE;
    __rw__ uint16_t REGMODE_PART[2];
    struct
    {
      __rw__ uint16_t REGMODE_L;
      __rw__ uint16_t REGMODE_H;
    };
  };
  __rw__ uint32_t OUTPUT;
  __rw__ uint32_t OUTPUTDIRCTRL;
  __rw__ uint32_t RES;
  union
  {
    __rw__ uint32_t DMAREQ[2];
    struct
    {
      __rw__ uint32_t DMAREQ0;
      __rw__ uint32_t DMAREQ1;
    };
  };
  __ne__ uint32_t RESERVED1[35];
  __rw__ uint32_t EVEN;
  __rw__ uint32_t EVFLAG;
  __rw__ uint32_t CONEN;
  __rw__ uint32_t CONFLAG;

  /* Offset 0x0100 */
  union
  {
    union
    {
      __rw__ uint32_t MATCH[16];
      __rw__ uint16_t MATCH_PART[16][2];
    };
    union
    {
      __ro__ uint32_t CAP[16];
      __ro__ uint16_t CAP_PART[16][2];
    };
  };
  union
  {
    __rw__ uint32_t FRACMAT[6];
    __rw__ uint16_t FRACMAT_PART[6][2];
  };
  __ne__ uint32_t RESERVED2[42]; /* Alias registers unused */
  union
  {
    union
    {
      __rw__ uint32_t MATCHREL[16];
      __rw__ uint16_t MATCHREL_PART[16][2];
    };
    union
    {
      __rw__ uint32_t CAPCTRL[16];
      __rw__ uint16_t CAPCTRL_PART[16][2];
    };
  };
  union
  {
    __rw__ uint32_t FRACMATREL[6];
    __rw__ uint16_t FRACMATREL_PART[6][2];
  };
  __ne__ uint32_t RESERVED3[42]; /* Alias registers unused */
  struct
  {
    __rw__ uint32_t STATE;
    __rw__ uint32_t CTRL;
  } EV[16];
  __ne__ uint32_t RESERVED4[96];
  struct
  {
    __rw__ uint32_t SET;
    __rw__ uint32_t CLR;
  } OUT[16];
} LPC_SCT_Type;
/*------------------System Configuration Registers----------------------------*/
typedef struct
{
  __rw__ uint32_t SYSMEMREMAP; /* System memory remap */
  __rw__ uint32_t PRESETCTRL; /* Peripheral reset control */
  __rw__ uint32_t SYSPLLCTRL; /* System PLL control */
  __rw__ uint32_t SYSPLLSTAT;
  __ne__ uint32_t RESERVED0[4];

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
  __ne__ uint32_t RESERVED3[10];
  __rw__ uint32_t MAINCLKSEL;
  __rw__ uint32_t MAINCLKUEN;
  __rw__ uint32_t SYSAHBCLKDIV;
  __ne__ uint32_t RESERVED4;

  /* Offset 0x0080 */
  __rw__ uint32_t SYSAHBCLKCTRL;
  __ne__ uint32_t RESERVED5[4];
  __rw__ uint32_t UARTCLKDIV;
  __ne__ uint32_t RESERVED6[18];

  /* Offset 0x00E0 */
  __rw__ uint32_t CLKOUTCLKSEL;
  __rw__ uint32_t CLKOUTUEN;
  __rw__ uint32_t CLKOUTDIV;
  __ne__ uint32_t RESERVED7;

  /* Offset 0x00F0 */
  __rw__ uint32_t UARTFRGDIV;
  __rw__ uint32_t UARTFRGMULT;
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t EXTTRACECMD;

  /* Offset 0x0100 */
  __rw__ uint32_t PIOPORCAP0;
  __ne__ uint32_t RESERVED10[12];
  __rw__ uint32_t IOCONCLKDIV[7];

  /* Offset 0x0150 */
  __rw__ uint32_t BODCTRL;
  __rw__ uint32_t SYSTCKCAL;
  __ne__ uint32_t RESERVED11[6];

  /* Offset 0x0170 */
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
  __ne__ uint32_t RESERVED12[26];

  /* Offset 0x0200 */
  __ne__ uint32_t RESERVED13;
  __rw__ uint32_t STARTERP0;
  __ne__ uint32_t RESERVED14[3];
  __rw__ uint32_t STARTERP1;
  __ne__ uint32_t RESERVED15[6];

  /* Offset 0x0230 */
  __rw__ uint32_t PDSLEEPCFG;
  __rw__ uint32_t PDAWAKECFG;
  __rw__ uint32_t PDRUNCFG;
  __ne__ uint32_t RESERVED16[111];
  __ro__ uint32_t DEVICE_ID;
} LPC_SYSCON_Type;
/*------------------Universal Synchronous Asynchronous Receiver Transmitter---*/
typedef struct
{
  __rw__ uint32_t CFG; /* Configuration register */
  __rw__ uint32_t CTL; /* Control register */
  __rw__ uint32_t STAT; /* Status register */
  __rw__ uint32_t INTENSET; /* Interrupt Enable Set register */
  __wo__ uint32_t INTENCLR; /* Interrupt Enable Clear register */
  __ro__ uint32_t RXDATA; /* Receive Data register */
  __ro__ uint32_t RXDATA_STAT; /* Receive Data with Status register */
  __rw__ uint32_t TXDATA; /* Transmit Data register */
  __rw__ uint32_t BRG; /* Baud Rate Generator register */
  __rw__ uint32_t INTSTAT; /* Interrupt Status register */
  __rw__ uint32_t OSR; /* Oversampling Selection Regiser */
  __rw__ uint32_t ADDR; /* Address register for automatic address matching */
} LPC_USART_Type;
/*------------------Self Wake-up Timer----------------------------------------*/
typedef struct
{
  __rw__ uint32_t CTRL;
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t COUNT;
} LPC_WKT_Type;
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
/*----------------------------------------------------------------------------*/
typedef struct
{
  LPC_WWDT_Type WWDT;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(LPC_WWDT_Type)];
  LPC_MRT_Type MRT;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(LPC_MRT_Type)];
  LPC_WKT_Type WKT;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(LPC_WKT_Type)];
  LPC_SWM_Type SWM;
  __ne__ uint8_t RESERVED3[0x10000 - sizeof(LPC_SWM_Type)];
  LPC_ADC_Type ADC;
  __ne__ uint8_t RESERVED4[0x4000 - sizeof(LPC_ADC_Type)];
  LPC_PMU_Type PMU;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(LPC_PMU_Type)];
  LPC_ACMP_Type ACMP;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(LPC_ACMP_Type)];
  LPC_DMA_INMUX_Type DMA_INMUX;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(LPC_DMA_INMUX_Type)];
  LPC_INMUX_Type INMUX;
  __ne__ uint8_t RESERVED8[0x14000 - sizeof(LPC_INMUX_Type)];
  LPC_FMC_Type FMC;
  __ne__ uint8_t RESERVED9[0x4000 - sizeof(LPC_FMC_Type)];
  LPC_IOCON_Type IOCON;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(LPC_IOCON_Type)];
  LPC_SYSCON_Type SYSCON;
  __ne__ uint8_t RESERVED11[0x8000 - sizeof(LPC_SYSCON_Type)];
  LPC_I2C_Type I2C0;
  __ne__ uint8_t RESERVED12[0x4000 - sizeof(LPC_I2C_Type)];
  LPC_I2C_Type I2C1;
  __ne__ uint8_t RESERVED13[0x4000 - sizeof(LPC_I2C_Type)];
  LPC_SPI_Type SPI0;
  __ne__ uint8_t RESERVED14[0x4000 - sizeof(LPC_SPI_Type)];
  LPC_SPI_Type SPI1;
  __ne__ uint8_t RESERVED15[0x8000 - sizeof(LPC_SPI_Type)];
  LPC_USART_Type USART0;
  __ne__ uint8_t RESERVED16[0x4000 - sizeof(LPC_USART_Type)];
  LPC_USART_Type USART1;
  __ne__ uint8_t RESERVED17[0x4000 - sizeof(LPC_USART_Type)];
  LPC_USART_Type USART2;
  __ne__ uint8_t RESERVED18[0x4000 - sizeof(LPC_USART_Type)];
  LPC_I2C_Type I2C2;
  __ne__ uint8_t RESERVED19[0x4000 - sizeof(LPC_I2C_Type)];
  LPC_I2C_Type I2C3;
} APB_DOMAIN_Type;

typedef struct
{
  LPC_GPIO_INT_Type CRC;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(LPC_GPIO_INT_Type)];
  LPC_SCT_Type SCT;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(LPC_SCT_Type)];
  LPC_SDMA_Type SDMA;
} AHB_DOMAIN_Type;

typedef struct
{
  LPC_GPIO_Type GPIO;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(LPC_GPIO_Type)];
  LPC_GPIO_INT_Type GPIO_INT;
} GPIO_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern APB_DOMAIN_Type APB_DOMAIN;
extern AHB_DOMAIN_Type AHB_DOMAIN;
extern GPIO_DOMAIN_Type GPIO_DOMAIN;
/*----------------------------------------------------------------------------*/
#define LPC_WWDT      (&APB_DOMAIN.WWDT)
#define LPC_WDT       (&APB_DOMAIN.WWDT.BASE)
#define LPC_MRT       (&APB_DOMAIN.MRT)
#define LPC_WKT       (&APB_DOMAIN.WKT)
#define LPC_SWM       (&APB_DOMAIN.SWM)
#define LPC_ADC       (&APB_DOMAIN.ADC)
#define LPC_PMU       (&APB_DOMAIN.PMU)
#define LPC_ACMP      (&APB_DOMAIN.ACMP)
#define LPC_DMA_INMUX (&APB_DOMAIN.DMA_INMUX)
#define LPC_INMUX     (&APB_DOMAIN.INMUX)
#define LPC_FMC       (&APB_DOMAIN.FMC)
#define LPC_IOCON     (&APB_DOMAIN.IOCON)
#define LPC_SYSCON    (&APB_DOMAIN.SYSCON)
#define LPC_I2C0      (&APB_DOMAIN.I2C0)
#define LPC_I2C1      (&APB_DOMAIN.I2C1)
#define LPC_SPI0      (&APB_DOMAIN.SPI0)
#define LPC_SPI1      (&APB_DOMAIN.SPI1)
#define LPC_USART0    (&APB_DOMAIN.USART0)
#define LPC_USART1    (&APB_DOMAIN.USART1)
#define LPC_USART2    (&APB_DOMAIN.USART2)
#define LPC_I2C2      (&APB_DOMAIN.I2C2)
#define LPC_I2C3      (&APB_DOMAIN.I2C3)

#define LPC_CRC       (&AHB_DOMAIN.CRC)
#define LPC_SCT       (&AHB_DOMAIN.SCT)
#define LPC_SDMA      (&AHB_DOMAIN.SDMA)

#define LPC_GPIO      (&GPIO_DOMAIN.GPIO)
#define LPC_GPIO_INT  (&GPIO_DOMAIN.GPIO_INT)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_PLATFORM_DEFS_H_ */
