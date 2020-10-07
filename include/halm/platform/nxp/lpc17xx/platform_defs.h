/*
 * halm/platform/nxp/lpc17xx/platform_defs.h
 * Based on original from NXP
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_LPC17XX_PLATFORM_DEFS_H_
#define HALM_PLATFORM_NXP_LPC17XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*----------------------------------------------------------------------------*/
#define NVIC_PRIORITY_SIZE 5
/*------------------System Control--------------------------------------------*/
typedef struct
{
  /* Flash Accelerator Module */
  __rw__ uint32_t FLASHCFG;
  __ne__ uint32_t RESERVED0[31];

  /* Phase Locked Loop 0, offset 0x80 */
  __rw__ uint32_t PLL0CON;
  __rw__ uint32_t PLL0CFG;
  __ro__ uint32_t PLL0STAT;
  __wo__ uint32_t PLL0FEED;
  __ne__ uint32_t RESERVED1[4];

  /* Phase Locked Loop 1, offset 0xA0 */
  __rw__ uint32_t PLL1CON;
  __rw__ uint32_t PLL1CFG;
  __ro__ uint32_t PLL1STAT;
  __wo__ uint32_t PLL1FEED;
  __ne__ uint32_t RESERVED2[4];

  /* Power control, offset 0xC0 */
  __rw__ uint32_t PCON;
  __rw__ uint32_t PCONP;
  __ne__ uint32_t RESERVED3[15];

  /* Clock dividers, offset 0x104 */
  __rw__ uint32_t CCLKCFG;
  __rw__ uint32_t USBCLKCFG;
  __rw__ uint32_t CLKSRCSEL;

  /* CAN Wake and Sleep registers, offset 0x110 */
  __rw__ uint32_t CANSLEEPCLR;
  __rw__ uint32_t CANWAKEFLAGS;
  __ne__ uint32_t RESERVED4[10];

  /* External interrupts, offset 0x140 */
  __rw__ uint32_t EXTINT;
  __ne__ uint32_t RESERVED5;
  __rw__ uint32_t EXTMODE;
  __rw__ uint32_t EXTPOLAR;
  __ne__ uint32_t RESERVED6[12];

  __rw__ uint32_t RSID; /* Reset Source Identification register */
  __ne__ uint32_t RESERVED7[7];
  __rw__ uint32_t SCS; /* System Control and Status */
  __ne__ uint32_t RESERVED8;

  /* Peripheral Clock Selection registers, offset 0x1A8 */
  __rw__ uint32_t PCLKSEL0;
  __rw__ uint32_t PCLKSEL1;
  __ne__ uint32_t RESERVED9[4];

  __rw__ uint32_t USBIntSt; /* USB Device/OTG Interrupt Status register */
  __rw__ uint32_t DMAREQSEL; /* DMA Request Select register */
  __rw__ uint32_t CLKOUTCFG; /* Clock Output Configuration register */
} LPC_SC_Type;
/*------------------Pin Connect Block-----------------------------------------*/
typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t PINSEL0;
      __rw__ uint32_t PINSEL1;
      __rw__ uint32_t PINSEL2;
      __rw__ uint32_t PINSEL3;
      __rw__ uint32_t PINSEL4;
      __rw__ uint32_t PINSEL5;
      __rw__ uint32_t PINSEL6;
      __rw__ uint32_t PINSEL7;
      __rw__ uint32_t PINSEL8;
      __rw__ uint32_t PINSEL9;
      __rw__ uint32_t PINSEL10;
    };

    __rw__ uint32_t PINSEL[11];
  };

  __ne__ uint32_t RESERVED0[5];

  union
  {
    struct
    {
      __rw__ uint32_t PINMODE0;
      __rw__ uint32_t PINMODE1;
      __rw__ uint32_t PINMODE2;
      __rw__ uint32_t PINMODE3;
      __rw__ uint32_t PINMODE4;
      __rw__ uint32_t PINMODE5;
      __rw__ uint32_t PINMODE6;
      __rw__ uint32_t PINMODE7;
      __rw__ uint32_t PINMODE8;
      __rw__ uint32_t PINMODE9;
    };

    __rw__ uint32_t PINMODE[10];
  };

  union
  {
    struct
    {
      __rw__ uint32_t PINMODE_OD0;
      __rw__ uint32_t PINMODE_OD1;
      __rw__ uint32_t PINMODE_OD2;
      __rw__ uint32_t PINMODE_OD3;
      __rw__ uint32_t PINMODE_OD4;
    };

    __rw__ uint32_t PINMODE_OD[5];
  };

  __rw__ uint32_t I2CPADCFG;
} LPC_PINCON_Type;
/*------------------General Purpose Input/Output------------------------------*/
typedef struct
{
  __rw__ uint32_t DIR;
  __ne__ uint32_t RESERVED0[3];
  __rw__ uint32_t MASK;
  __rw__ uint32_t PIN;
  __rw__ uint32_t SET;
  __wo__ uint32_t CLR;
} LPC_GPIO_Type;

typedef struct
{
  __ro__ uint32_t STATUS;
  union
  {
    struct
    {
      __ro__ uint32_t STATR;
      __ro__ uint32_t STATF;
      __wo__ uint32_t CLR;
      __rw__ uint32_t ENR;
      __rw__ uint32_t ENF;
      __ne__ uint32_t RESERVED[3];
    } PORT[2];

    struct
    {
      __ro__ uint32_t STATR0;
      __ro__ uint32_t STATF0;
      __wo__ uint32_t CLR0;
      __rw__ uint32_t ENR0;
      __rw__ uint32_t ENF0;
      __ne__ uint32_t RESERVED0[3];
      __ro__ uint32_t STATR2;
      __ro__ uint32_t STATF2;
      __wo__ uint32_t CLR2;
      __rw__ uint32_t ENR2;
      __rw__ uint32_t ENF2;
      __ne__ uint32_t RESERVED1[3];
    };
  };

} LPC_GPIO_INT_Type;
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
  __rw__ uint32_t TRIM; /* May be unavailable on some parts */
} LPC_ADC_Type;
/*------------------Digital-to-Analog Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR; /* Converter Register */
  __rw__ uint32_t CTRL; /* Control register */
  __rw__ uint16_t CNTVAL; /* Counter Value register */
} LPC_DAC_Type;
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
  __rw__ uint32_t DMACR;
} LPC_SSP_Type;
/*------------------Serial Peripheral Interface-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __ro__ uint32_t SR;
  __rw__ uint32_t DR;
  __rw__ uint32_t CCR;
  __ne__ uint32_t RESERVED[3];
  __rw__ uint32_t INT;
} LPC_SPI_Type;
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
      __ro__ uint32_t CR1;
      __ro__ uint32_t CR2;
      __ro__ uint32_t CR3;
    };
  };
  __rw__ uint32_t EMR;
  __ne__ uint32_t RESERVED1[12];
  __rw__ uint32_t CTCR;
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
  __ne__ uint32_t RESERVED0;
  __ro__ uint32_t LSR;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t SCR;
  __rw__ uint32_t ACR;
  __rw__ uint32_t ICR;
  __rw__ uint32_t FDR;
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t TER;
} LPC_UART_Type;
/*------------------Universal Synchronous Asynchronous Receiver Transmitter---*/
typedef struct
{
  union
  {
    struct
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
      __rw__ uint32_t RESERVED0;
      __rw__ uint32_t FDR;
      __ne__ uint32_t RESERVED1;
      __rw__ uint32_t TER;
    };

    LPC_UART_Type BASE;
  };

  __ne__ uint32_t RESERVED2[6];
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t RS485ADRMATCH;
  __rw__ uint32_t RS485DLY;
} LPC_USART_Type;
/*------------------Pulse-Width Modulator-------------------------------------*/
typedef struct
{
  __rw__ uint32_t IR;
  __rw__ uint32_t TCR;
  __rw__ uint32_t TC;
  __rw__ uint32_t PR;
  __rw__ uint32_t PC;
  __rw__ uint32_t MCR;
  __rw__ uint32_t MR0;
  __rw__ uint32_t MR1;
  __rw__ uint32_t MR2;
  __rw__ uint32_t MR3;
  __rw__ uint32_t CCR;
  __ro__ uint32_t CR0;
  __ro__ uint32_t CR1;
  __ro__ uint32_t CR2;
  __ro__ uint32_t CR3;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t MR4;
  __rw__ uint32_t MR5;
  __rw__ uint32_t MR6;
  __rw__ uint32_t PCR;
  __rw__ uint32_t LER;
  __ne__ uint32_t RESERVED1[7];
  __rw__ uint32_t CTCR;
} LPC_PWM_Type;
/*------------------Inter IC Sound--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t DAO;
  __rw__ uint32_t DAI;
  __wo__ uint32_t TXFIFO;
  __ro__ uint32_t RXFIFO;
  __ro__ uint32_t STATE;
  __rw__ uint32_t DMA1;
  __rw__ uint32_t DMA2;
  __rw__ uint32_t IRQ;
  __rw__ uint32_t TXRATE;
  __rw__ uint32_t RXRATE;
  __rw__ uint32_t TXBITRATE;
  __rw__ uint32_t RXBITRATE;
  __rw__ uint32_t TXMODE;
  __rw__ uint32_t RXMODE;
} LPC_I2S_Type;
/*------------------Repetitive Interrupt Timer--------------------------------*/
typedef struct
{
  __rw__ uint32_t COMPVAL;
  __rw__ uint32_t MASK;
  __rw__ uint32_t CTRL;
  __rw__ uint32_t COUNTER;
} LPC_RIT_Type;
/*------------------Real-Time Clock-------------------------------------------*/
typedef struct
{
  /* Miscellaneous registers, offset 0x00 */
  __rw__ uint32_t ILR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t CCR;
  __rw__ uint32_t CIIR;
  __rw__ uint32_t AMR;

  /* Consolidated time registers, offset 0x14 */
  __ro__ uint32_t CTIME0;
  __ro__ uint32_t CTIME1;
  __ro__ uint32_t CTIME2;

  /* Time counter registers, offset 0x20 */
  __rw__ uint32_t SEC;
  __rw__ uint32_t MIN;
  __rw__ uint32_t HOUR;
  __rw__ uint32_t DOM;
  __rw__ uint32_t DOW;
  __rw__ uint32_t DOY;
  __rw__ uint32_t MONTH;
  __rw__ uint32_t YEAR;
  __rw__ uint32_t CALIBRATION;

  /* General purpose registers, offset 0x44 */
  union
  {
    __rw__ uint32_t GPREG[5];
    struct
    {
      __rw__ uint32_t GPREG0;
      __rw__ uint32_t GPREG1;
      __rw__ uint32_t GPREG2;
      __rw__ uint32_t GPREG3;
      __rw__ uint32_t GPREG4;
    };
  };

  /* Miscellaneous registers, offset 0x5C */
  __rw__ uint32_t RTC_AUXEN;
  __rw__ uint32_t RTC_AUX;

  /* Alarm register group, offset 0x60 */
  __rw__ uint32_t ALSEC;
  __rw__ uint32_t ALMIN;
  __rw__ uint32_t ALHOUR;
  __rw__ uint32_t ALDOM;
  __rw__ uint32_t ALDOW;
  __rw__ uint32_t ALDOY;
  __rw__ uint32_t ALMON;
  __rw__ uint32_t ALYEAR;
} LPC_RTC_Type;
/*------------------Watchdog Timer--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t MOD;
  __rw__ uint32_t TC;
  __wo__ uint32_t FEED;
  __ro__ uint32_t TV;
  __rw__ uint32_t CLKSEL;
} LPC_WDT_Type;
/*------------------Motor Control Pulse-Width Modulation----------------------*/
typedef struct
{
  __ro__ uint32_t CON;
  __wo__ uint32_t CON_SET;
  __wo__ uint32_t CON_CLR;
  __ro__ uint32_t CAPCON;
  __wo__ uint32_t CAPCON_SET;
  __wo__ uint32_t CAPCON_CLR;
  __rw__ uint32_t TC0;
  __rw__ uint32_t TC1;
  __rw__ uint32_t TC2;
  __rw__ uint32_t LIM0;
  __rw__ uint32_t LIM1;
  __rw__ uint32_t LIM2;
  __rw__ uint32_t MAT0;
  __rw__ uint32_t MAT1;
  __rw__ uint32_t MAT2;
  __rw__ uint32_t DT;

  /* Offset 0x40 */
  __rw__ uint32_t MCCP;
  __ro__ uint32_t CAP0;
  __ro__ uint32_t CAP1;
  __ro__ uint32_t CAP2;
  __ro__ uint32_t INTEN;
  __wo__ uint32_t INTEN_SET;
  __wo__ uint32_t INTEN_CLR;
  __ro__ uint32_t CNTCON;
  __wo__ uint32_t CNTCON_SET;
  __wo__ uint32_t CNTCON_CLR;
  __ro__ uint32_t INTF;
  __wo__ uint32_t INTF_SET;
  __wo__ uint32_t INTF_CLR;
  __wo__ uint32_t CAP_CLR;
} LPC_MCPWM_Type;
/*------------------Quadrature Encoder Interface------------------------------*/
typedef struct
{
  __wo__ uint32_t CON;
  __ro__ uint32_t STAT;
  __rw__ uint32_t CONF;
  __ro__ uint32_t POS;
  __rw__ uint32_t MAXPOS;
  __rw__ uint32_t CMPOS0;
  __rw__ uint32_t CMPOS1;
  __rw__ uint32_t CMPOS2;

  /* Offset 0x0020 */
  __ro__ uint32_t INXCNT;
  __rw__ uint32_t INXCMP;
  __rw__ uint32_t LOAD;
  __ro__ uint32_t TIME;
  __ro__ uint32_t VEL;
  __ro__ uint32_t CAP;
  __rw__ uint32_t VELCOMP;
  __rw__ uint32_t FILTER;
  __ne__ uint32_t RESERVED0[998];

  /* Offset 0x0FD8 */
  __wo__ uint32_t IEC;
  __wo__ uint32_t IES;
  __ro__ uint32_t INTSTAT;
  __ro__ uint32_t IE;
  __wo__ uint32_t CLR;
  __wo__ uint32_t SET;
} LPC_QEI_Type;
/*------------------Controller Area Network-----------------------------------*/
/* Identifier masks */
typedef struct
{
  __rw__ uint32_t MASK[512];
} LPC_CANAF_RAM_Type;

/* Acceptance Filter registers */
typedef struct
{
  __rw__ uint32_t AFMR;
  __rw__ uint32_t SFF_sa;
  __rw__ uint32_t SFF_GRP_sa;
  __rw__ uint32_t EFF_sa;
  __rw__ uint32_t EFF_GRP_sa;
  __rw__ uint32_t ENDofTable;
  __ro__ uint32_t LUTerrAd;
  __ro__ uint32_t LUTerr;
  __rw__ uint32_t FCANIE;
  __rw__ uint32_t FCANIC0;
  __rw__ uint32_t FCANIC1;
} LPC_CANAF_Type;

/* Central registers */
typedef struct
{
  __ro__ uint32_t TxSR;
  __ro__ uint32_t RxSR;
  __ro__ uint32_t MSR;
} LPC_CANCR_Type;

/* Controller registers */
typedef struct
{
  __rw__ uint32_t MOD;
  __wo__ uint32_t CMR;
  __rw__ uint32_t GSR;
  __ro__ uint32_t ICR;
  __rw__ uint32_t IER;
  __rw__ uint32_t BTR;
  __rw__ uint32_t EWL;
  __ro__ uint32_t SR;

  __rw__ uint32_t RFS;
  __rw__ uint32_t RID;
  __rw__ uint32_t RDA;
  __rw__ uint32_t RDB;

  union
  {
    struct
    {
      __rw__ uint32_t TFI1;
      __rw__ uint32_t TID1;
      __rw__ uint32_t TDA1;
      __rw__ uint32_t TDB1;
      __rw__ uint32_t TFI2;
      __rw__ uint32_t TID2;
      __rw__ uint32_t TDA2;
      __rw__ uint32_t TDB2;
      __rw__ uint32_t TFI3;
      __rw__ uint32_t TID3;
      __rw__ uint32_t TDA3;
      __rw__ uint32_t TDB3;
    };

    struct
    {
      __rw__ uint32_t TFI;
      __rw__ uint32_t TID;
      __rw__ uint32_t TDA;
      __rw__ uint32_t TDB;
    } TX[3];
  };
} LPC_CAN_Type;
/*------------------General Purpose Direct Memory Access controller-----------*/
/* Channel registers */
typedef struct
{
  __rw__ uint32_t SRCADDR;
  __rw__ uint32_t DESTADDR;
  __rw__ uint32_t LLI;
  __rw__ uint32_t CONTROL;
  __rw__ uint32_t CONFIG;
  __ne__ uint32_t RESERVED[3];
} LPC_GPDMA_CHANNEL_Type;

/* Common registers */
typedef struct
{
  __ro__ uint32_t INTSTAT;
  __ro__ uint32_t INTTCSTAT;
  __wo__ uint32_t INTTCCLEAR;
  __ro__ uint32_t INTERRSTAT;
  __wo__ uint32_t INTERRCLEAR;
  __ro__ uint32_t RAWINTTCSTAT;
  __ro__ uint32_t RAWINTERRSTAT;
  __ro__ uint32_t ENBLDCHNS;
  __rw__ uint32_t SOFTBREQ;
  __rw__ uint32_t SOFTSREQ;
  __rw__ uint32_t SOFTLBREQ;
  __rw__ uint32_t SOFTLSREQ;
  __rw__ uint32_t CONFIG;
  __rw__ uint32_t SYNC;
  __ne__ uint32_t RESERVED[50];

  /* Offset 0x100: channel registers */
  LPC_GPDMA_CHANNEL_Type CHANNELS[8];
} LPC_GPDMA_Type;
/*------------------Universal Serial Bus--------------------------------------*/
typedef struct
{
  /* USB Host registers */
  __ro__ uint32_t HcRevision;
  __rw__ uint32_t HcControl;
  __rw__ uint32_t HcCommandStatus;
  __rw__ uint32_t HcInterruptStatus;
  __rw__ uint32_t HcInterruptEnable;
  __rw__ uint32_t HcInterruptDisable;
  __rw__ uint32_t HcHCCA;
  __ro__ uint32_t HcPeriodCurrentED;
  __rw__ uint32_t HcControlHeadED;
  __rw__ uint32_t HcControlCurrentED;
  __rw__ uint32_t HcBulkHeadED;
  __rw__ uint32_t HcBulkCurrentED;
  __ro__ uint32_t HcDoneHead;
  __rw__ uint32_t HcFmInterval;
  __ro__ uint32_t HcFmRemaining;
  __ro__ uint32_t HcFmNumber;
  __rw__ uint32_t HcPeriodicStart;
  __rw__ uint32_t HcLSTreshold;
  __rw__ uint32_t HcRhDescriptorA;
  __rw__ uint32_t HcRhDescriptorB;
  __rw__ uint32_t HcRhStatus;
  __rw__ uint32_t HcRhPortStatus1;
  __rw__ uint32_t HcRhPortStatus2;
  __ne__ uint32_t RESERVED0[40];
  __ro__ uint32_t Module_ID;

  /* USB On-The-Go registers */
  __ro__ uint32_t OTGIntSt;
  __rw__ uint32_t OTGIntEn;
  __wo__ uint32_t OTGIntSet;
  __wo__ uint32_t OTGIntClr;
  __rw__ uint32_t OTGStCtrl;
  __rw__ uint32_t OTGTmr;
  __ne__ uint32_t RESERVED1[58];

  /* USB Device Interrupt registers */
  __ro__ uint32_t USBDevIntSt;
  __rw__ uint32_t USBDevIntEn;
  __wo__ uint32_t USBDevIntClr;
  __wo__ uint32_t USBDevIntSet;

  /* USB Device SIE Command registers */
  __wo__ uint32_t USBCmdCode;
  __ro__ uint32_t USBCmdData;

  /* USB Device Transfer registers */
  __ro__ uint32_t USBRxData;
  __wo__ uint32_t USBTxData;
  __ro__ uint32_t USBRxPLen;
  __wo__ uint32_t USBTxPLen;
  __rw__ uint32_t USBCtrl;
  __wo__ uint32_t USBDevIntPri;

  /* USB Device Endpoint Interrupt registers */
  __ro__ uint32_t USBEpIntSt;
  __rw__ uint32_t USBEpIntEn;
  __wo__ uint32_t USBEpIntClr;
  __wo__ uint32_t USBEpIntSet;
  __wo__ uint32_t USBEpIntPri;

  /* USB Device Endpoint Realization registers */
  __rw__ uint32_t USBReEp;
  __wo__ uint32_t USBEpInd;
  __rw__ uint32_t USBMaxPSize;

  /* USB Device DMA registers */
  __ro__ uint32_t USBDMARSt;
  __wo__ uint32_t USBDMARClr;
  __wo__ uint32_t USBDMARSet;
  __ne__ uint32_t RESERVED2[9];
  __rw__ uint32_t USBUDCAH;
  __ro__ uint32_t USBEpDMASt;
  __wo__ uint32_t USBEpDMAEn;
  __wo__ uint32_t USBEpDMADis;
  __ro__ uint32_t USBDMAIntSt;
  __rw__ uint32_t USBDMAIntEn;
  __ne__ uint32_t RESERVED3[2];
  __ro__ uint32_t USBEoTIntSt;
  __wo__ uint32_t USBEoTIntClr;
  __wo__ uint32_t USBEoTIntSet;
  __ro__ uint32_t USBNDDRIntSt;
  __wo__ uint32_t USBNDDRIntClr;
  __wo__ uint32_t USBNDDRIntSet;
  __ro__ uint32_t USBSysErrIntSt;
  __wo__ uint32_t USBSysErrIntClr;
  __wo__ uint32_t USBSysErrIntSet;
  __ne__ uint32_t RESERVED4[15];

  /* USB OTG I2C registers */
  union
  {
    __ro__ uint32_t I2C_RX;
    __wo__ uint32_t I2C_TX;
  };
  __ro__ uint32_t I2C_STS;
  __rw__ uint32_t I2C_CTL;
  __rw__ uint32_t I2C_CLKHI;
  __wo__ uint32_t I2C_CLKLO;
  __ne__ uint32_t RESERVED5[824];

  /* USB Clock Control registers */
  union
  {
    __rw__ uint32_t USBClkCtrl;
    __rw__ uint32_t OTGClkCtrl;
  };
  union
  {
    __ro__ uint32_t USBClkSt;
    __ro__ uint32_t OTGClkSt;
  };
} LPC_USB_Type;
/*------------------Ethernet Media Access Controller--------------------------*/
typedef struct
{
  /* MAC registers */
  __rw__ uint32_t MAC1;
  __rw__ uint32_t MAC2;
  __rw__ uint32_t IPGT;
  __rw__ uint32_t IPGR;
  __rw__ uint32_t CLRT;
  __rw__ uint32_t MAXF;
  __rw__ uint32_t SUPP;
  __rw__ uint32_t TEST;
  __rw__ uint32_t MCFG;
  __rw__ uint32_t MCMD;
  __rw__ uint32_t MADR;
  __wo__ uint32_t MWTD;
  __ro__ uint32_t MRDD;
  __ro__ uint32_t MIND;
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t SA0;
  __rw__ uint32_t SA1;
  __rw__ uint32_t SA2;
  __ne__ uint32_t RESERVED1[45];

  /* Control registers */
  __rw__ uint32_t Command;
  __ro__ uint32_t Status;
  __rw__ uint32_t RxDescriptor;
  __rw__ uint32_t RxStatus;
  __rw__ uint32_t RxDescriptorNumber;
  __ro__ uint32_t RxProduceIndex;
  __rw__ uint32_t RxConsumeIndex;
  __rw__ uint32_t TxDescriptor;
  __rw__ uint32_t TxStatus;
  __rw__ uint32_t TxDescriptorNumber;
  __rw__ uint32_t TxProduceIndex;
  __ro__ uint32_t TxConsumeIndex;
  __ne__ uint32_t RESERVED2[10];
  __ro__ uint32_t TSV0;
  __ro__ uint32_t TSV1;
  __ro__ uint32_t RSV;
  __ne__ uint32_t RESERVED3[3];
  __rw__ uint32_t FlowControlCounter;
  __ro__ uint32_t FlowControlStatus;
  __ne__ uint32_t RESERVED4[34];

  /* Rx Filter registers */
  __rw__ uint32_t RxFilterCtrl;
  __rw__ uint32_t RxFilterWoLStatus;
  __rw__ uint32_t RxFilterWoLClear;
  __ne__ uint32_t RESERVED5;
  __rw__ uint32_t HashFilterL;
  __rw__ uint32_t HashFilterH;
  __ne__ uint32_t RESERVED6[882];

  /* Module Control registers */
  __ro__ uint32_t IntStatus;
  __rw__ uint32_t IntEnable;
  __wo__ uint32_t IntClear;
  __wo__ uint32_t IntSet;
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t PowerDown;
} LPC_ETHERNET_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  LPC_WDT_Type WDT;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(LPC_WDT_Type)];
  LPC_TIMER_Type TIMER0;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_TIMER_Type TIMER1;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_UART_Type UART0;
  __ne__ uint8_t RESERVED3[0x4000 - sizeof(LPC_UART_Type)];
  LPC_USART_Type UART1;
  __ne__ uint8_t RESERVED4[0x8000 - sizeof(LPC_USART_Type)];
  LPC_PWM_Type PWM1;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(LPC_PWM_Type)];
  LPC_I2C_Type I2C0;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(LPC_I2C_Type)];
  LPC_SPI_Type SPI;
  __ne__ uint8_t RESERVED7[0x4000 - sizeof(LPC_SPI_Type)];
  LPC_RTC_Type RTC;
  __ne__ uint8_t RESERVED8[0x4080 - sizeof(LPC_RTC_Type)];
  LPC_GPIO_INT_Type GPIO_INT;
  __ne__ uint8_t RESERVED9[0x3F80 - sizeof(LPC_GPIO_INT_Type)];
  LPC_PINCON_Type PINCON;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(LPC_PINCON_Type)];
  LPC_SSP_Type SSP1;
  __ne__ uint8_t RESERVED11[0x4000 - sizeof(LPC_SSP_Type)];
  LPC_ADC_Type ADC;
  __ne__ uint8_t RESERVED12[0x4000 - sizeof(LPC_ADC_Type)];
  LPC_CANAF_RAM_Type CANAF_RAM;
  __ne__ uint8_t RESERVED13[0x4000 - sizeof(LPC_CANAF_RAM_Type)];
  LPC_CANAF_Type CANAF;
  __ne__ uint8_t RESERVED14[0x4000 - sizeof(LPC_CANAF_Type)];
  LPC_CANCR_Type CANCR;
  __ne__ uint8_t RESERVED15[0x4000 - sizeof(LPC_CANCR_Type)];
  LPC_CAN_Type CAN1;
  __ne__ uint8_t RESERVED16[0x4000 - sizeof(LPC_CAN_Type)];
  LPC_CAN_Type CAN2;
  __ne__ uint8_t RESERVED17[0x14000 - sizeof(LPC_CAN_Type)];
  LPC_I2C_Type I2C1;
} APB0_DOMAIN_Type;

typedef struct
{
  __ne__ uint8_t RESERVED0[0x8000];
  LPC_SSP_Type SSP0;
  __ne__ uint8_t RESERVED1[0x4000 - sizeof(LPC_SSP_Type)];
  LPC_DAC_Type DAC;
  __ne__ uint8_t RESERVED2[0x4000 - sizeof(LPC_DAC_Type)];
  LPC_TIMER_Type TIMER2;
  __ne__ uint8_t RESERVED3[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_TIMER_Type TIMER3;
  __ne__ uint8_t RESERVED4[0x4000 - sizeof(LPC_TIMER_Type)];
  LPC_UART_Type UART2;
  __ne__ uint8_t RESERVED5[0x4000 - sizeof(LPC_UART_Type)];
  LPC_UART_Type UART3;
  __ne__ uint8_t RESERVED6[0x4000 - sizeof(LPC_UART_Type)];
  LPC_I2C_Type I2C2;
  __ne__ uint8_t RESERVED7[0x8000 - sizeof(LPC_I2C_Type)];
  LPC_I2S_Type I2S;
  __ne__ uint8_t RESERVED8[0x8000 - sizeof(LPC_I2S_Type)];
  LPC_RIT_Type RIT;
  __ne__ uint8_t RESERVED9[0x8000 - sizeof(LPC_RIT_Type)];
  LPC_MCPWM_Type MCPWM;
  __ne__ uint8_t RESERVED10[0x4000 - sizeof(LPC_MCPWM_Type)];
  LPC_QEI_Type QEI;
  __ne__ uint8_t RESERVED11[0x40000 - sizeof(LPC_QEI_Type)];
  LPC_SC_Type SC;
} APB1_DOMAIN_Type;

typedef struct
{
  LPC_ETHERNET_Type ETHERNET;
  __ne__ uint8_t RESERVED0[0x4000 - sizeof(LPC_ETHERNET_Type)];
  LPC_GPDMA_Type GPDMA;
  __ne__ uint8_t RESERVED1[0x8000 - sizeof(LPC_GPDMA_Type)];
  LPC_USB_Type USB;
} AHB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern LPC_GPIO_Type    GPIO_DOMAIN[5];
extern APB0_DOMAIN_Type APB0_DOMAIN;
extern APB1_DOMAIN_Type APB1_DOMAIN;
extern AHB_DOMAIN_Type  AHB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define LPC_GPIO      (GPIO_DOMAIN)
#define LPC_GPIO0     (&GPIO_DOMAIN[0])
#define LPC_GPIO1     (&GPIO_DOMAIN[1])
#define LPC_GPIO2     (&GPIO_DOMAIN[2])
#define LPC_GPIO3     (&GPIO_DOMAIN[3])
#define LPC_GPIO4     (&GPIO_DOMAIN[4])

#define LPC_WDT       (&APB0_DOMAIN.WDT)
#define LPC_TIMER0    (&APB0_DOMAIN.TIMER0)
#define LPC_TIMER1    (&APB0_DOMAIN.TIMER1)
#define LPC_UART0     (&APB0_DOMAIN.UART0)
#define LPC_UART1     (&APB0_DOMAIN.UART1)
#define LPC_PWM1      (&APB0_DOMAIN.PWM1)
#define LPC_I2C0      (&APB0_DOMAIN.I2C0)
#define LPC_SPI       (&APB0_DOMAIN.SPI)
#define LPC_RTC       (&APB0_DOMAIN.RTC)
#define LPC_GPIO_INT  (&APB0_DOMAIN.GPIO_INT)
#define LPC_PINCON    (&APB0_DOMAIN.PINCON)
#define LPC_SSP1      (&APB0_DOMAIN.SSP1)
#define LPC_ADC       (&APB0_DOMAIN.ADC)
#define LPC_CANAF_RAM (&APB0_DOMAIN.CANAF_RAM)
#define LPC_CANAF     (&APB0_DOMAIN.CANAF)
#define LPC_CANCR     (&APB0_DOMAIN.CANCR)
#define LPC_CAN1      (&APB0_DOMAIN.CAN1)
#define LPC_CAN2      (&APB0_DOMAIN.CAN2)
#define LPC_I2C1      (&APB0_DOMAIN.I2C1)

#define LPC_SSP0      (&APB1_DOMAIN.SSP0)
#define LPC_DAC       (&APB1_DOMAIN.DAC)
#define LPC_TIMER2    (&APB1_DOMAIN.TIMER2)
#define LPC_TIMER3    (&APB1_DOMAIN.TIMER3)
#define LPC_UART2     (&APB1_DOMAIN.UART2)
#define LPC_UART3     (&APB1_DOMAIN.UART3)
#define LPC_I2C2      (&APB1_DOMAIN.I2C2)
#define LPC_I2S       (&APB1_DOMAIN.I2S)
#define LPC_RIT       (&APB1_DOMAIN.RIT)
#define LPC_MCPWM     (&APB1_DOMAIN.MCPWM)
#define LPC_QEI       (&APB1_DOMAIN.QEI)
#define LPC_SC        (&APB1_DOMAIN.SC)

#define LPC_ETHERNET  (&AHB_DOMAIN.ETHERNET)
#define LPC_GPDMA     (&AHB_DOMAIN.GPDMA)
#define LPC_USB       (&AHB_DOMAIN.USB)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_LPC17XX_PLATFORM_DEFS_H_ */
