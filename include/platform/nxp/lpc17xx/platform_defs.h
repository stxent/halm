/*
 * platform/nxp/lpc17xx/platform_defs.h
 * Based on original by NXP
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_LPC17XX_PLATFORM_DEFS_H_
#define PLATFORM_NXP_LPC17XX_PLATFORM_DEFS_H_
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
  __ne__ uint32_t RESERVED0[5];
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
  __rw__ uint32_t PINMODE_OD0;
  __rw__ uint32_t PINMODE_OD1;
  __rw__ uint32_t PINMODE_OD2;
  __rw__ uint32_t PINMODE_OD3;
  __rw__ uint32_t PINMODE_OD4;
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
      __ro__ uint32_t CR1;
    };
  };
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t EMR;
  __ne__ uint32_t RESERVED1[12];
  __rw__ uint32_t CTCR;
  __rw__ uint32_t PWMC; /* Chip-specific register */
} LPC_TIMER_Type;
/*------------------Universal Asynchronous Receiver Transmitter---------------*/
typedef struct
{
  union {
    __ro__ uint32_t RBR;
    __wo__ uint32_t THR;
    __rw__ uint32_t DLL;
  };
  union {
    __rw__ uint32_t DLM;
    __rw__ uint32_t IER;
  };
  union {
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
/*------------------Extended Universal Asynchronous Receiver Transmitter------*/
/* UART block with modem control, RS485 support and IrDA mode */
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
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t TER;
  __ne__ uint32_t RESERVED1[6];
  __rw__ uint32_t RS485CTRL;
  __rw__ uint32_t RS485ADRMATCH;
  __rw__ uint32_t RS485DLY;
} LPC_UART_MODEM_Type;
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
  __rw__ uint32_t I2SDAO;
  __rw__ uint32_t I2SDAI;
  __wo__ uint32_t I2STXFIFO;
  __ro__ uint32_t I2SRXFIFO;
  __ro__ uint32_t I2SSTATE;
  __rw__ uint32_t I2SDMA1;
  __rw__ uint32_t I2SDMA2;
  __rw__ uint32_t I2SIRQ;
  __rw__ uint32_t I2STXRATE;
  __rw__ uint32_t I2SRXRATE;
  __rw__ uint32_t I2STXBITRATE;
  __rw__ uint32_t I2SRXBITRATE;
  __rw__ uint32_t I2STXMODE;
  __rw__ uint32_t I2SRXMODE;
} LPC_I2S_Type;
/*------------------Repetitive Interrupt Timer--------------------------------*/
typedef struct
{
  __rw__ uint32_t RICOMPVAL;
  __rw__ uint32_t RIMASK;
  __rw__ uint32_t RICTRL;
  __rw__ uint32_t RICOUNTER;
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
  __rw__ uint32_t GPREG0;
  __rw__ uint32_t GPREG1;
  __rw__ uint32_t GPREG2;
  __rw__ uint32_t GPREG3;
  __rw__ uint32_t GPREG4;

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
  __rw__ uint32_t WDCLKSEL;
} LPC_WDT_Type;
/*------------------Motor Control Pulse-Width Modulation----------------------*/
typedef struct
{
  __ro__ uint32_t MCCON;
  __wo__ uint32_t MCCON_SET;
  __wo__ uint32_t MCCON_CLR;
  __ro__ uint32_t MCCAPCON;
  __wo__ uint32_t MCCAPCON_SET;
  __wo__ uint32_t MCCAPCON_CLR;
  __rw__ uint32_t MCTIM0;
  __rw__ uint32_t MCTIM1;
  __rw__ uint32_t MCTIM2;
  __rw__ uint32_t MCPER0;
  __rw__ uint32_t MCPER1;
  __rw__ uint32_t MCPER2;
  __rw__ uint32_t MCPW0;
  __rw__ uint32_t MCPW1;
  __rw__ uint32_t MCPW2;
  __rw__ uint32_t MCDEADTIME;
  __rw__ uint32_t MCCCP;
  __rw__ uint32_t MCCR0;
  __rw__ uint32_t MCCR1;
  __rw__ uint32_t MCCR2;
  __ro__ uint32_t MCINTEN;
  __wo__ uint32_t MCINTEN_SET;
  __wo__ uint32_t MCINTEN_CLR;
  __ro__ uint32_t MCCNTCON;
  __wo__ uint32_t MCCNTCON_SET;
  __wo__ uint32_t MCCNTCON_CLR;
  __ro__ uint32_t MCINTFLAG;
  __wo__ uint32_t MCINTFLAG_SET;
  __wo__ uint32_t MCINTFLAG_CLR;
  __wo__ uint32_t MCCAP_CLR;
} LPC_MCPWM_Type;
/*------------------Quadrature Encoder Interface------------------------------*/
typedef struct
{
  __wo__ uint32_t QEICON;
  __ro__ uint32_t QEISTAT;
  __rw__ uint32_t QEICONF;
  __ro__ uint32_t QEIPOS;
  __rw__ uint32_t QEIMAXPOS;
  __rw__ uint32_t CMPOS0;
  __rw__ uint32_t CMPOS1;
  __rw__ uint32_t CMPOS2;
  __ro__ uint32_t INXCNT;
  __rw__ uint32_t INXCMP;
  __rw__ uint32_t QEILOAD;
  __ro__ uint32_t QEITIME;
  __ro__ uint32_t QEIVEL;
  __ro__ uint32_t QEICAP;
  __rw__ uint32_t VELCOMP;
  __rw__ uint32_t FILTER;
  __ne__ uint32_t RESERVED0[998];
  __wo__ uint32_t QEIIEC;
  __wo__ uint32_t QEIIES;
  __ro__ uint32_t QEIINTSTAT;
  __ro__ uint32_t QEIIE;
  __wo__ uint32_t QEICLR;
  __wo__ uint32_t QEISET;
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
  __rw__ uint32_t SFF_SA;
  __rw__ uint32_t SFF_GRP_SA;
  __rw__ uint32_t EFF_SA;
  __rw__ uint32_t EFF_GRP_SA;
  __rw__ uint32_t ENDOFTABLE;
  __ro__ uint32_t LUTERRAD;
  __ro__ uint32_t LUTERR;
  __rw__ uint32_t FCANIE;
  __rw__ uint32_t FCANIC0;
  __rw__ uint32_t FCANIC1;
} LPC_CANAF_Type;

/* Central registers */
typedef struct
{
  __ro__ uint32_t TXSR;
  __ro__ uint32_t RXSR;
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
} LPC_CAN_Type;
/*------------------General Purpose Direct Memory Access----------------------*/
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
} LPC_GPDMA_Type;

/* Channel registers */
typedef struct
{
  __rw__ uint32_t SRCADDR;
  __rw__ uint32_t DESTADDR;
  __rw__ uint32_t LLI;
  __rw__ uint32_t CONTROL;
  __rw__ uint32_t CONFIG;
} LPC_GPDMACH_Type;
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
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t Module_ID;
} LPC_ETHERNET_Type;
/*----------------------------------------------------------------------------*/
/* Base addresses */
#define LPC_GPIO_BASE           (0x2009C000UL)
#define LPC_APB0_BASE           (0x40000000UL)
#define LPC_APB1_BASE           (0x40080000UL)
#define LPC_AHB_BASE            (0x50000000UL)

/* APB0 peripherals */
#define LPC_WDT_BASE            (LPC_APB0_BASE + 0x00000)
#define LPC_TIMER0_BASE         (LPC_APB0_BASE + 0x04000)
#define LPC_TIMER1_BASE         (LPC_APB0_BASE + 0x08000)
#define LPC_UART0_BASE          (LPC_APB0_BASE + 0x0C000)
#define LPC_UART1_BASE          (LPC_APB0_BASE + 0x10000)
#define LPC_PWM1_BASE           (LPC_APB0_BASE + 0x18000)
#define LPC_I2C0_BASE           (LPC_APB0_BASE + 0x1C000)
#define LPC_SPI_BASE            (LPC_APB0_BASE + 0x20000)
#define LPC_RTC_BASE            (LPC_APB0_BASE + 0x24000)
#define LPC_GPIO_INT_BASE       (LPC_APB0_BASE + 0x28080)
#define LPC_PINCON_BASE         (LPC_APB0_BASE + 0x2C000)
#define LPC_SSP1_BASE           (LPC_APB0_BASE + 0x30000)
#define LPC_ADC_BASE            (LPC_APB0_BASE + 0x34000)
#define LPC_CANAF_RAM_BASE      (LPC_APB0_BASE + 0x38000)
#define LPC_CANAF_BASE          (LPC_APB0_BASE + 0x3C000)
#define LPC_CANCR_BASE          (LPC_APB0_BASE + 0x40000)
#define LPC_CAN1_BASE           (LPC_APB0_BASE + 0x44000)
#define LPC_CAN2_BASE           (LPC_APB0_BASE + 0x48000)
#define LPC_I2C1_BASE           (LPC_APB0_BASE + 0x5C000)

/* APB1 peripherals */
#define LPC_SSP0_BASE           (LPC_APB1_BASE + 0x08000)
#define LPC_DAC_BASE            (LPC_APB1_BASE + 0x0C000)
#define LPC_TIMER2_BASE         (LPC_APB1_BASE + 0x10000)
#define LPC_TIMER3_BASE         (LPC_APB1_BASE + 0x14000)
#define LPC_UART2_BASE          (LPC_APB1_BASE + 0x18000)
#define LPC_UART3_BASE          (LPC_APB1_BASE + 0x1C000)
#define LPC_I2C2_BASE           (LPC_APB1_BASE + 0x20000)
#define LPC_I2S_BASE            (LPC_APB1_BASE + 0x28000)
#define LPC_RIT_BASE            (LPC_APB1_BASE + 0x30000)
#define LPC_MCPWM_BASE          (LPC_APB1_BASE + 0x38000)
#define LPC_QEI_BASE            (LPC_APB1_BASE + 0x3C000)
#define LPC_SC_BASE             (LPC_APB1_BASE + 0x7C000)

/* AHB peripherals */
#define LPC_ETHERNET_BASE       (LPC_AHB_BASE + 0x00000)
#define LPC_GPDMA_BASE          (LPC_AHB_BASE + 0x04000)
#define LPC_GPDMACH0_BASE       (LPC_AHB_BASE + 0x04100)
#define LPC_GPDMACH1_BASE       (LPC_AHB_BASE + 0x04120)
#define LPC_GPDMACH2_BASE       (LPC_AHB_BASE + 0x04140)
#define LPC_GPDMACH3_BASE       (LPC_AHB_BASE + 0x04160)
#define LPC_GPDMACH4_BASE       (LPC_AHB_BASE + 0x04180)
#define LPC_GPDMACH5_BASE       (LPC_AHB_BASE + 0x041A0)
#define LPC_GPDMACH6_BASE       (LPC_AHB_BASE + 0x041C0)
#define LPC_GPDMACH7_BASE       (LPC_AHB_BASE + 0x041E0)
#define LPC_USB_BASE            (LPC_AHB_BASE + 0x0C000)

/* General Purpose Input/Output */
#define LPC_GPIO0_BASE          (LPC_GPIO_BASE + 0x00000)
#define LPC_GPIO1_BASE          (LPC_GPIO_BASE + 0x00020)
#define LPC_GPIO2_BASE          (LPC_GPIO_BASE + 0x00040)
#define LPC_GPIO3_BASE          (LPC_GPIO_BASE + 0x00060)
#define LPC_GPIO4_BASE          (LPC_GPIO_BASE + 0x00080)
/*----------------------------------------------------------------------------*/
/* Peripheral declaration */
#define LPC_SC          ((LPC_SC_Type *)LPC_SC_BASE)
#define LPC_ADC         ((LPC_ADC_Type *)LPC_ADC_BASE)
#define LPC_CAN1        ((LPC_CAN_Type *)LPC_CAN1_BASE)
#define LPC_CAN2        ((LPC_CAN_Type *)LPC_CAN2_BASE)
#define LPC_CANAF       ((LPC_CANAF_Type *)LPC_CANAF_BASE)
#define LPC_CANAF_RAM   ((LPC_CANAF_RAM_Type *)LPC_CANAF_RAM_BASE)
#define LPC_CANCR       ((LPC_CANCR_Type *)LPC_CANCR_BASE)
#define LPC_DAC         ((LPC_DAC_Type *)LPC_DAC_BASE)
#define LPC_ETHERNET    ((LPC_ETHERNET_Type *)LPC_ETHERNET_BASE)
#define LPC_GPDMA       ((LPC_GPDMA_Type *)LPC_GPDMA_BASE)
#define LPC_GPDMACH0    ((LPC_GPDMACH_Type *)LPC_GPDMACH0_BASE)
#define LPC_GPDMACH1    ((LPC_GPDMACH_Type *)LPC_GPDMACH1_BASE)
#define LPC_GPDMACH2    ((LPC_GPDMACH_Type *)LPC_GPDMACH2_BASE)
#define LPC_GPDMACH3    ((LPC_GPDMACH_Type *)LPC_GPDMACH3_BASE)
#define LPC_GPDMACH4    ((LPC_GPDMACH_Type *)LPC_GPDMACH4_BASE)
#define LPC_GPDMACH5    ((LPC_GPDMACH_Type *)LPC_GPDMACH5_BASE)
#define LPC_GPDMACH6    ((LPC_GPDMACH_Type *)LPC_GPDMACH6_BASE)
#define LPC_GPDMACH7    ((LPC_GPDMACH_Type *)LPC_GPDMACH7_BASE)
#define LPC_GPIO0       ((LPC_GPIO_Type *)LPC_GPIO0_BASE)
#define LPC_GPIO1       ((LPC_GPIO_Type *)LPC_GPIO1_BASE)
#define LPC_GPIO2       ((LPC_GPIO_Type *)LPC_GPIO2_BASE)
#define LPC_GPIO3       ((LPC_GPIO_Type *)LPC_GPIO3_BASE)
#define LPC_GPIO4       ((LPC_GPIO_Type *)LPC_GPIO4_BASE)
#define LPC_GPIO_INT    ((LPC_GPIO_INT_Type *)LPC_GPIO_INT_BASE)
#define LPC_I2C0        ((LPC_I2C_Type *)LPC_I2C0_BASE)
#define LPC_I2C1        ((LPC_I2C_Type *)LPC_I2C1_BASE)
#define LPC_I2C2        ((LPC_I2C_Type *)LPC_I2C2_BASE)
#define LPC_I2S         ((LPC_I2S_Type *)LPC_I2S_BASE)
#define LPC_MCPWM       ((LPC_MCPWM_Type *)LPC_MCPWM_BASE)
#define LPC_PINCON      ((LPC_PINCON_Type *)LPC_PINCON_BASE)
#define LPC_PWM1        ((LPC_PWM_Type *)LPC_PWM1_BASE)
#define LPC_QEI         ((LPC_QEI_Type *)LPC_QEI_BASE)
#define LPC_RIT         ((LPC_RIT_Type *)LPC_RIT_BASE)
#define LPC_RTC         ((LPC_RTC_Type *)LPC_RTC_BASE)
#define LPC_SPI         ((LPC_SPI_Type *)LPC_SPI_BASE)
#define LPC_SSP0        ((LPC_SSP_Type *)LPC_SSP0_BASE)
#define LPC_SSP1        ((LPC_SSP_Type *)LPC_SSP1_BASE)
#define LPC_TIMER0      ((LPC_TIMER_Type *)LPC_TIMER0_BASE)
#define LPC_TIMER1      ((LPC_TIMER_Type *)LPC_TIMER1_BASE)
#define LPC_TIMER2      ((LPC_TIMER_Type *)LPC_TIMER2_BASE)
#define LPC_TIMER3      ((LPC_TIMER_Type *)LPC_TIMER3_BASE)
#define LPC_UART0       ((LPC_UART_Type *)LPC_UART0_BASE)
#define LPC_UART1       ((LPC_UART_MODEM_Type *)LPC_UART1_BASE)
#define LPC_UART2       ((LPC_UART_Type *)LPC_UART2_BASE)
#define LPC_UART3       ((LPC_UART_Type *)LPC_UART3_BASE)
#define LPC_USB         ((LPC_USB_Type *)LPC_USB_BASE)
#define LPC_WDT         ((LPC_WDT_Type *)LPC_WDT_BASE)
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_LPC17XX_PLATFORM_DEFS_H_ */
