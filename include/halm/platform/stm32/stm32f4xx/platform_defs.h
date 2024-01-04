/*
 * halm/platform/stm32/stm32f4xx/platform_defs.h
 * Based on original from ST
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F4XX_PLATFORM_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F4XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------Analog to Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t SR;
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;

  union
  {
    struct
    {
      __rw__ uint32_t SMPR1;
      __rw__ uint32_t SMPR2;
    };

    __rw__ uint32_t SMPR[2];
  };

  union
  {
    struct
    {
      __rw__ uint32_t JOFR1;
      __rw__ uint32_t JOFR2;
      __rw__ uint32_t JOFR3;
      __rw__ uint32_t JOFR4;
    };

    __rw__ uint32_t JOFR[4];
  };

  __rw__ uint32_t HTR;
  __rw__ uint32_t LTR;

  union
  {
    struct
    {
      __rw__ uint32_t SQR1;
      __rw__ uint32_t SQR2;
      __rw__ uint32_t SQR3;
    };

    __rw__ uint32_t SQR[3];
  };

  __rw__ uint32_t JSQR;

  union
  {
    struct
    {
      __rw__ uint32_t JDR1;
      __rw__ uint32_t JDR2;
      __rw__ uint32_t JDR3;
      __rw__ uint32_t JDR4;
    };

    __rw__ uint32_t JDR[4];
  };

  __rw__ uint32_t DR;
} STM_ADC_Type;

typedef struct
{
  __rw__ uint32_t CSR;
  __rw__ uint32_t CCR;
  __rw__ uint32_t CDR;
} STM_ADC_COMMON_Type;
/*------------------Controller Area Network-----------------------------------*/
typedef struct
{
  __rw__ uint32_t FR1;
  __rw__ uint32_t FR2;
} STM_CAN_FILTER_Type;

typedef struct
{
  __rw__ uint32_t MCR;
  __rw__ uint32_t MSR;
  __rw__ uint32_t TSR;

  union
  {
    struct
    {
      __rw__ uint32_t RF0R;
      __rw__ uint32_t RF1R;
    };

    __rw__ uint32_t RFR[2];
  };

  __rw__ uint32_t IER;
  __rw__ uint32_t ESR;
  __rw__ uint32_t BTR;
  __ne__ uint32_t RESERVED0[88];

  /* Offset 0x180: TX mailboxes */
  struct
  {
    __rw__ uint32_t TIR;
    __rw__ uint32_t TDTR;
    __rw__ uint32_t TDLR;
    __rw__ uint32_t TDHR;
  } TX[3];

  /* Offset 0x1B0: RX FIFO mailboxes */
  struct
  {
    __rw__ uint32_t RIR;
    __rw__ uint32_t RDTR;
    __rw__ uint32_t RDLR;
    __rw__ uint32_t RDHR;
  } RX_FIFO[2];

  __ne__ uint32_t RESERVED1[12];
  __rw__ uint32_t FMR;
  __rw__ uint32_t FM1R;
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t FS1R;
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t FFA1R;
  __ne__ uint32_t RESERVED4;
  __rw__ uint32_t FA1R;
  __ne__ uint32_t RESERVED5[8];

  /* Offset 0x240: filter registers, 14 for CL parts, 28 for other parts */
  STM_CAN_FILTER_Type FILTERS[28];
} STM_CAN_Type;
/*------------------CRC calculation unit--------------------------------------*/
typedef struct
{
  __rw__ uint32_t DR;
  __rw__ uint32_t IDR;
  __rw__ uint32_t CR;
} STM_CRC_Type;
/*------------------Digital to Analog Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __wo__ uint32_t SWTRIGR;
  __rw__ uint32_t DHR12R1;
  __rw__ uint32_t DHR12L1;
  __rw__ uint32_t DHR8R1;
  __rw__ uint32_t DHR12R2;
  __rw__ uint32_t DHR12L2;
  __rw__ uint32_t DHR8R2;
  __rw__ uint32_t DHR12RD;
  __rw__ uint32_t DHR12LD;
  __rw__ uint32_t DHR8RD;
  __ro__ uint32_t DOR1;
  __ro__ uint32_t DOR2;
  __rw__ uint32_t SR;
} STM_DAC_Type;
/*------------------Digital Camera Interface----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t SR;
  __rw__ uint32_t RISR;
  __rw__ uint32_t IER;
  __rw__ uint32_t MISR;
  __rw__ uint32_t ICR;
  __rw__ uint32_t ESCR;
  __rw__ uint32_t ESUR;
  __rw__ uint32_t CWSTRTR;
  __rw__ uint32_t CWSIZER;
  __rw__ uint32_t DR;
} STM_DCMI_Type;
/*------------------Debug MCU-------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t IDCODE;
  __rw__ uint32_t CR;
  __rw__ uint32_t APB1FZ;
  __rw__ uint32_t APB2FZ;
} STM_DBGMCU_Type;
/*------------------DMA Controller--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t NDTR;
  __rw__ uint32_t PAR;
  __rw__ uint32_t M0AR;
  __rw__ uint32_t M1AR;
  __rw__ uint32_t FCR;
} STM_DMA_STREAM_Type;

typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t LISR;
      __rw__ uint32_t HISR;
    };

    __rw__ uint32_t ISR[2];
  };

  union
  {
    struct
    {
      __rw__ uint32_t LIFCR;
      __rw__ uint32_t HIFCR;
    };

    __rw__ uint32_t IFCR[2];
  };

  /* Offset 0x10: stream registers */
  STM_DMA_STREAM_Type STREAMS[8];
} STM_DMA_Type;
/*------------------Ethernet MAC----------------------------------------------*/
typedef struct
{
  __rw__ uint32_t MACCR;
  __rw__ uint32_t MACFFR;
  __rw__ uint32_t MACHTHR;
  __rw__ uint32_t MACHTLR;
  __rw__ uint32_t MACMIIAR;
  __rw__ uint32_t MACMIIDR;
  __rw__ uint32_t MACFCR;
  __rw__ uint32_t MACVLANTR;
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t MACRWUFFR;
  __rw__ uint32_t MACPMTCSR;
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t MACSR;
  __rw__ uint32_t MACIMR;
  __rw__ uint32_t MACA0HR;
  __rw__ uint32_t MACA0LR;
  __rw__ uint32_t MACA1HR;
  __rw__ uint32_t MACA1LR;
  __rw__ uint32_t MACA2HR;
  __rw__ uint32_t MACA2LR;
  __rw__ uint32_t MACA3HR;
  __rw__ uint32_t MACA3LR;
  __ne__ uint32_t RESERVED2[40];
  __rw__ uint32_t MMCCR;
  __rw__ uint32_t MMCRIR;
  __rw__ uint32_t MMCTIR;
  __rw__ uint32_t MMCRIMR;
  __rw__ uint32_t MMCTIMR;
  __ne__ uint32_t RESERVED3[14];
  __rw__ uint32_t MMCTGFSCCR;
  __rw__ uint32_t MMCTGFMSCCR;
  __ne__ uint32_t RESERVED4[5];
  __rw__ uint32_t MMCTGFCR;
  __ne__ uint32_t RESERVED5[10];
  __rw__ uint32_t MMCRFCECR;
  __rw__ uint32_t MMCRFAECR;
  __ne__ uint32_t RESERVED6[10];
  __rw__ uint32_t MMCRGUFCR;
  __ne__ uint32_t RESERVED7[334];
  __rw__ uint32_t PTPTSCR;
  __rw__ uint32_t PTPSSIR;
  __rw__ uint32_t PTPTSHR;
  __rw__ uint32_t PTPTSLR;
  __rw__ uint32_t PTPTSHUR;
  __rw__ uint32_t PTPTSLUR;
  __rw__ uint32_t PTPTSAR;
  __rw__ uint32_t PTPTTHR;
  __rw__ uint32_t PTPTTLR;
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t PTPTSSR;
  __ne__ uint32_t RESERVED9[565];
  __rw__ uint32_t DMABMR;
  __rw__ uint32_t DMATPDR;
  __rw__ uint32_t DMARPDR;
  __rw__ uint32_t DMARDLAR;
  __rw__ uint32_t DMATDLAR;
  __rw__ uint32_t DMASR;
  __rw__ uint32_t DMAOMR;
  __rw__ uint32_t DMAIER;
  __rw__ uint32_t DMAMFBOCR;
  __rw__ uint32_t DMARSWTR;
  __ne__ uint32_t RESERVED10[8];
  __rw__ uint32_t DMACHTDR;
  __rw__ uint32_t DMACHRDR;
  __rw__ uint32_t DMACHTBAR;
  __rw__ uint32_t DMACHRBAR;
} STM_ETHERNET_Type;
/*------------------External Interrupt/Event Controller-----------------------*/
typedef struct
{
  __rw__ uint32_t IMR;
  __rw__ uint32_t EMR;
  __rw__ uint32_t RTSR;
  __rw__ uint32_t FTSR;
  __rw__ uint32_t SWIER;
  __rw__ uint32_t PR;
} STM_EXTI_Type;
/*------------------Flash interface-------------------------------------------*/
typedef struct
{
  __rw__ uint32_t ACR;
  __rw__ uint32_t KEYR;
  __rw__ uint32_t OPTKEYR;
  __rw__ uint32_t SR;
  __rw__ uint32_t CR;
  __rw__ uint32_t OPTCR;
} STM_FLASH_Type;
/*------------------Flexible Static Memory Controller-------------------------*/
// TODO Verify
typedef struct
{
  __rw__ uint32_t BCR1;
  __rw__ uint32_t BTR1;
  __rw__ uint32_t BCR2;
  __rw__ uint32_t BTR2;
  __rw__ uint32_t BCR3;
  __rw__ uint32_t BTR3;
  __rw__ uint32_t BCR4;
  __rw__ uint32_t BTR4;
  __ne__ uint32_t RESERVED0[16];

  /* Offset 0x060 */
  __rw__ uint32_t PCR2;
  __rw__ uint32_t SR2;
  __rw__ uint32_t PMEM2;
  __rw__ uint32_t PATT2;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t ECCR2;
  __ne__ uint32_t RESERVED2[2];

  /* Offset 0x080 */
  __rw__ uint32_t PCR3;
  __rw__ uint32_t SR3;
  __rw__ uint32_t PMEM3;
  __rw__ uint32_t PATT3;
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t ECCR3;
  __ne__ uint32_t RESERVED4[2];

  /* Offset 0x0A0 */
  __rw__ uint32_t PCR4;
  __rw__ uint32_t SR4;
  __rw__ uint32_t PMEM4;
  __rw__ uint32_t PATT4;
  __rw__ uint32_t PIO4;
  __ne__ uint32_t RESERVED5[20];

  /* Offset 0x104 */
  __rw__ uint32_t BWTR1;
  __ne__ uint32_t RESERVED6;
  __rw__ uint32_t BWTR2;
  __ne__ uint32_t RESERVED7;
  __rw__ uint32_t BWTR3;
  __ne__ uint32_t RESERVED8;
  __rw__ uint32_t BWTR4;
} STM_FSMC_Type;
/*------------------General Purpose Input/Output------------------------------*/
typedef struct
{
  __rw__ uint32_t MODER;
  __rw__ uint32_t OTYPER;
  __rw__ uint32_t OSPEEDR;
  __rw__ uint32_t PUPDR;
  __ro__ uint32_t IDR;
  __rw__ uint32_t ODR;
  __rw__ uint32_t BSRR;
  __rw__ uint32_t LCKR;
  __rw__ uint32_t AFR[2];
  __ne__ uint32_t RESERVED0[246];
} STM_GPIO_Type;
/*------------------System Configuration controller---------------------------*/
typedef struct
{
  __rw__ uint32_t MEMRMP;
  __rw__ uint32_t PMC;
  __rw__ uint32_t EXTICR[4];
  __ne__ uint32_t RESERVED[2];
  __rw__ uint32_t CMPCR;
} STM_SYSCFG_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t OAR1;
  __rw__ uint32_t OAR2;
  __rw__ uint32_t DR;
  __rw__ uint32_t SR1;
  __rw__ uint32_t SR2;
  __rw__ uint32_t CCR;
  __rw__ uint32_t TRISE;
} STM_I2C_Type;
/*------------------Independent Watchdog--------------------------------------*/
typedef struct
{
  __wo__ uint32_t KR;
  __rw__ uint32_t PR;
  __rw__ uint32_t RLR;
  __ro__ uint32_t SR;
} STM_IWDG_Type;
/*------------------Power Control---------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t CSR;
} STM_PWR_Type;
/*------------------Reset and Clock Control-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t PLLCFGR;
  __rw__ uint32_t CFGR;
  __rw__ uint32_t CIR;
  __rw__ uint32_t AHB1RSTR;
  __rw__ uint32_t AHB2RSTR;
  __rw__ uint32_t AHB3RSTR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t APB1RSTR;
  __rw__ uint32_t APB2RSTR;
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t AHB1ENR;
  __rw__ uint32_t AHB2ENR;
  __rw__ uint32_t AHB3ENR;
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t APB1ENR;
  __rw__ uint32_t APB2ENR;
  __ne__ uint32_t RESERVED3[2];
  __rw__ uint32_t AHB1LPENR;
  __rw__ uint32_t AHB2LPENR;
  __rw__ uint32_t AHB3LPENR;
  __ne__ uint32_t RESERVED4;
  __rw__ uint32_t APB1LPENR;
  __rw__ uint32_t APB2LPENR;
  __ne__ uint32_t RESERVED5[2];
  __rw__ uint32_t BDCR;
  __rw__ uint32_t CSR;
  __ne__ uint32_t RESERVED6[2];
  __rw__ uint32_t SSCGR;
  __rw__ uint32_t PLLI2SCFGR;
} STM_RCC_Type;
/*------------------Real-Time Clock-------------------------------------------*/
typedef struct
{
  __rw__ uint32_t TR;
  __rw__ uint32_t DR;
  __rw__ uint32_t CR;
  __rw__ uint32_t ISR;
  __rw__ uint32_t PRER;
  __rw__ uint32_t WUTR;
  __rw__ uint32_t CALIBR;
  __rw__ uint32_t ALRMAR;
  __rw__ uint32_t ALRMBR;
  __rw__ uint32_t WPR;
  __rw__ uint32_t SSR;
  __rw__ uint32_t SHIFTR;
  __rw__ uint32_t TSTR;
  __rw__ uint32_t TSDR;
  __rw__ uint32_t TSSSR;
  __rw__ uint32_t CALR;
  __rw__ uint32_t TAFCR;
  __rw__ uint32_t ALRMASSR;
  __rw__ uint32_t ALRMBSSR;
  __ne__ uint32_t RESERVED0;

  /* Offset 0x50 */
  __rw__ uint32_t BKPR[20];
} STM_RTC_Type;
/*------------------SD/MMC card interface-------------------------------------*/
typedef struct
{
  __rw__ uint32_t POWER;
  __rw__ uint32_t CLKCR;
  __rw__ uint32_t ARG;
  __rw__ uint32_t CMD;
  __ro__ uint32_t RESPCMD;

  union
  {
    struct
    {
      __ro__ uint32_t RESP1;
      __ro__ uint32_t RESP2;
      __ro__ uint32_t RESP3;
      __ro__ uint32_t RESP4;
    };

    __ro__ uint32_t RESP[4];
  };

  __rw__ uint32_t DTIMER;
  __rw__ uint32_t DLEN;
  __rw__ uint32_t DCTRL;
  __ro__ uint32_t DCOUNT;
  __ro__ uint32_t STA;
  __rw__ uint32_t ICR;
  __rw__ uint32_t MASK;
  __ne__ uint32_t RESERVED0[2];
  __ro__ uint32_t FIFOCNT;
  __ne__ uint32_t RESERVED1[13];
  __rw__ uint32_t FIFO;
} STM_SDIO_Type;
/*------------------Serial Peripheral Interface-------------------------------*/
typedef struct
{
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t SR;
  __rw__ uint32_t DR;
  __rw__ uint32_t CRCPR;
  __ro__ uint32_t RXCRCR;
  __ro__ uint32_t TXCRCR;
  __rw__ uint32_t I2SCFGR;
  __rw__ uint32_t I2SPR;
} STM_SPI_Type;
/*------------------Timer-----------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t SMCR;
  __rw__ uint32_t DIER;
  __rw__ uint32_t SR;
  __wo__ uint32_t EGR;

  union
  {
    struct
    {
      __rw__ uint32_t CCMR1;
      __rw__ uint32_t CCMR2;
    };

    __rw__ uint32_t CCMR[2];
  };

  __rw__ uint32_t CCER;
  __rw__ uint32_t CNT;
  __rw__ uint32_t PSC;
  __rw__ uint32_t ARR;
  __rw__ uint32_t RCR;

  union
  {
    struct
    {
      __rw__ uint32_t CCR1;
      __rw__ uint32_t CCR2;
      __rw__ uint32_t CCR3;
      __rw__ uint32_t CCR4;
    };

    __rw__ uint32_t CCR[4];
  };

  __rw__ uint32_t BDTR;
  __rw__ uint32_t DCR;
  __rw__ uint32_t DMAR;
  __rw__ uint32_t OR;
} STM_TIM_Type;
/*------------------Universal Synchronous Asynchronous Receiver Transmitter---*/
typedef struct
{
  __rw__ uint32_t SR;
  __rw__ uint32_t DR;
  __rw__ uint32_t BRR;
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t CR3;
  __rw__ uint32_t GTPR;
} STM_USART_Type;
/*------------------Windowed Watchdog-----------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t CFR;
  __rw__ uint32_t SR;
} STM_WWDG_Type;
/*------------------Cryptographic processor-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t SR;
  __rw__ uint32_t DR;
  __rw__ uint32_t DOUT;
  __rw__ uint32_t DMACR;
  __rw__ uint32_t IMSCR;
  __rw__ uint32_t RISR;
  __rw__ uint32_t MISR;
  __rw__ uint32_t K0LR;
  __rw__ uint32_t K0RR;
  __rw__ uint32_t K1LR;
  __rw__ uint32_t K1RR;
  __rw__ uint32_t K2LR;
  __rw__ uint32_t K2RR;
  __rw__ uint32_t K3LR;
  __rw__ uint32_t K3RR;
  __rw__ uint32_t IV0LR;
  __rw__ uint32_t IV0RR;
  __rw__ uint32_t IV1LR;
  __rw__ uint32_t IV1RR;
} STM_CRYP_Type;
/*------------------Hash processor--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t DIN;
  __rw__ uint32_t STR;
  __rw__ uint32_t HR[5];
  __rw__ uint32_t IMR;
  __rw__ uint32_t SR;
  __ne__ uint32_t RESERVED0[52];
  __rw__ uint32_t CSR[51];
} STM_HASH_Type;
/*------------------Random Number Generator-----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t SR;
  __rw__ uint32_t DR;
} STM_RNG_Type;
/*------------------USB OTG---------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t GOTGCTL;
  __rw__ uint32_t GOTGINT;
  __rw__ uint32_t GAHBCFG;
  __rw__ uint32_t GUSBCFG;
  __rw__ uint32_t GRSTCTL;
  __rw__ uint32_t GINTSTS;
  __rw__ uint32_t GINTMSK;
  __rw__ uint32_t GRXSTSR;
  __rw__ uint32_t GRXSTSP;
  __rw__ uint32_t GRXFSIZ;

  union
  {
    __rw__ uint32_t DIEPTXF0;
    __rw__ uint32_t HNPTXFSIZ;
  };

  union
  {
    __rw__ uint32_t GNPTXSTS;
    __rw__ uint32_t HNPTXSTS;
  };

  __rw__ uint32_t GI2CCTL;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t GCCFG;
  __rw__ uint32_t CID;
  __ne__ uint32_t RESERVED1[48];
  __rw__ uint32_t HPTXFSIZ;
  __rw__ uint32_t DIEPTXF[15];
  __ne__ uint32_t RESERVED2[176];
} STM_OTG_GLOBAL_Type;

typedef struct
{
  __rw__ uint32_t HCFG;
  __rw__ uint32_t HFIR;
  __rw__ uint32_t HFNUM;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t HPTXSTS;
  __rw__ uint32_t HAINT;
  __rw__ uint32_t HAINTMSK;
  __ne__ uint32_t RESERVED1[9];
  __rw__ uint32_t HPRT;
  __ne__ uint32_t RESERVED2[47];
} STM_OTG_HOST_Type;

typedef struct
{
  __rw__ uint32_t HCCHAR;
  __rw__ uint32_t HCSPLT;
  __rw__ uint32_t HCINT;
  __rw__ uint32_t HCINTMSK;
  __rw__ uint32_t HCTSIZ;
  __rw__ uint32_t HCDMA;
  __ne__ uint32_t RESERVED0[2];
} STM_OTG_HOST_CH_Type;

typedef struct
{
  __rw__ uint32_t DCFG;
  __rw__ uint32_t DCTL;
  __rw__ uint32_t DSTS;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t DIEPMSK;
  __rw__ uint32_t DOEPMSK;
  __rw__ uint32_t DAINT;
  __rw__ uint32_t DAINTMSK;
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t DVBUSDIS;
  __rw__ uint32_t DVBUSPULSE;
  __rw__ uint32_t DTHRCTL;
  __rw__ uint32_t DIEPEMPMSK;
  __rw__ uint32_t DEACHINT;
  __rw__ uint32_t DEACHMSK;
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t DINEP1MSK;
  __ne__ uint32_t RESERVED3[15];
  __rw__ uint32_t DOUTEP1MSK;
  __ne__ uint32_t RESERVED4[30];
} STM_OTG_DEV_Type;

typedef struct
{
  __rw__ uint32_t DIEPCTL;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t DIEPINT;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t DIEPTSIZ;
  __rw__ uint32_t DIEPDMA;
  __rw__ uint32_t DTXFSTS;
  __ne__ uint32_t RESERVED2;
} STM_OTG_EP_IN_Type;

typedef struct
{
  __rw__ uint32_t DOEPCTL;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t DOEPINT;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t DOEPTSIZ;
  __rw__ uint32_t DOEPDMA;
  __ne__ uint32_t RESERVED2[2];
} STM_OTG_EP_OUT_Type;

typedef struct
{
  __rw__ uint32_t PCGCCTL;
  __ne__ uint32_t RESERVED0[63];
} STM_OTG_PCG_Type;

typedef struct
{
  __rw__ uint32_t DATA[1024];
} STM_OTG_FIFO_Type;

typedef struct
{
  STM_OTG_GLOBAL_Type GLOBAL;

  /* Offset 0x400 */
  STM_OTG_HOST_Type HOST;
  /* Offset 0x500 */
  STM_OTG_HOST_CH_Type HOST_CH[16];
  /* Offset 0x700 */
  __ne__ uint8_t RESERVED1[256];

  /* Offset 0x800 */
  STM_OTG_DEV_Type DEV;
  /* Offset 0x900 */
  STM_OTG_EP_IN_Type DEV_EP_IN[8];
  /* Offset 0xA00 */
  __ne__ uint8_t RESERVED2[256];
  /* Offset 0xB00 */
  STM_OTG_EP_OUT_Type DEV_EP_OUT[8];
  /* Offset 0xC00 */
  __ne__ uint8_t RESERVED3[512];

  /* Offset 0xE00 */
  STM_OTG_PCG_Type PCG;
  __ne__ uint8_t RESERVED4[256];

  /* Offset 0x1000 */
  STM_OTG_FIFO_Type FIFO[16];
} STM_USB_OTG_Type;
/*----------------------------------------------------------------------------*/
typedef struct
{
  STM_TIM_Type TIM2;
  __ne__ uint8_t RESERVED0[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM3;
  __ne__ uint8_t RESERVED1[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM4;
  __ne__ uint8_t RESERVED2[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM5;
  __ne__ uint8_t RESERVED3[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM6;
  __ne__ uint8_t RESERVED4[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM7;
  __ne__ uint8_t RESERVED5[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM12;
  __ne__ uint8_t RESERVED6[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM13;
  __ne__ uint8_t RESERVED7[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM14;
  __ne__ uint8_t RESERVED8[0x800 - sizeof(STM_TIM_Type)];
  STM_RTC_Type RTC;
  __ne__ uint8_t RESERVED9[0x400 - sizeof(STM_RTC_Type)];
  STM_WWDG_Type WWDG;
  __ne__ uint8_t RESERVED10[0x400 - sizeof(STM_WWDG_Type)];
  STM_IWDG_Type IWDG;
  __ne__ uint8_t RESERVED11[0x400 - sizeof(STM_IWDG_Type)];
  STM_SPI_Type SPI2EXT;
  __ne__ uint8_t RESERVED12[0x400 - sizeof(STM_SPI_Type)];
  STM_SPI_Type SPI2;
  __ne__ uint8_t RESERVED13[0x400 - sizeof(STM_SPI_Type)];
  STM_SPI_Type SPI3;
  __ne__ uint8_t RESERVED14[0x400 - sizeof(STM_SPI_Type)];
  STM_SPI_Type SPI3EXT;
  __ne__ uint8_t RESERVED15[0x400 - sizeof(STM_SPI_Type)];
  STM_USART_Type USART2;
  __ne__ uint8_t RESERVED16[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART3;
  __ne__ uint8_t RESERVED17[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type UART4;
  __ne__ uint8_t RESERVED18[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type UART5;
  __ne__ uint8_t RESERVED19[0x400 - sizeof(STM_USART_Type)];
  STM_I2C_Type I2C1;
  __ne__ uint8_t RESERVED20[0x400 - sizeof(STM_I2C_Type)];
  STM_I2C_Type I2C2;
  __ne__ uint8_t RESERVED21[0x400 - sizeof(STM_I2C_Type)];
  STM_I2C_Type I2C3;
  __ne__ uint8_t RESERVED22[0x800 - sizeof(STM_I2C_Type)];
  STM_CAN_Type CAN1;
  __ne__ uint8_t RESERVED23[0x400 - sizeof(STM_CAN_Type)];
  STM_CAN_Type CAN2;
  __ne__ uint8_t RESERVED24[0x800 - sizeof(STM_CAN_Type)];
  STM_PWR_Type PWR;
  __ne__ uint8_t RESERVED25[0x400 - sizeof(STM_PWR_Type)];
  STM_DAC_Type DAC;
} APB1_DOMAIN_Type;

typedef struct
{
  STM_TIM_Type TIM1;
  __ne__ uint8_t RESERVED0[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM8;
  __ne__ uint8_t RESERVED1[0xC00 - sizeof(STM_TIM_Type)];
  STM_USART_Type USART1;
  __ne__ uint8_t RESERVED2[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART6;
  __ne__ uint8_t RESERVED3[0xC00 - sizeof(STM_USART_Type)];
  STM_ADC_Type ADC1;
  __ne__ uint8_t RESERVED4[0x100 - sizeof(STM_ADC_Type)];
  STM_ADC_Type ADC2;
  __ne__ uint8_t RESERVED5[0x100 - sizeof(STM_ADC_Type)];
  STM_ADC_Type ADC3;
  __ne__ uint8_t RESERVED6[0x100 - sizeof(STM_ADC_Type)];
  STM_ADC_COMMON_Type ADC_COMMON;
  __ne__ uint8_t RESERVED7[0x900 - sizeof(STM_ADC_COMMON_Type)];
  STM_SDIO_Type SDIO;
  __ne__ uint8_t RESERVED8[0x400 - sizeof(STM_SDIO_Type)];
  STM_SPI_Type SPI1;
  __ne__ uint8_t RESERVED9[0x800 - sizeof(STM_SPI_Type)];
  STM_SYSCFG_Type SYSCFG;
  __ne__ uint8_t RESERVED10[0x400 - sizeof(STM_SYSCFG_Type)];
  STM_EXTI_Type EXTI;
  __ne__ uint8_t RESERVED11[0x400 - sizeof(STM_EXTI_Type)];
  STM_TIM_Type TIM9;
  __ne__ uint8_t RESERVED12[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM10;
  __ne__ uint8_t RESERVED13[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM11;
} APB2_DOMAIN_Type;

typedef struct
{
  STM_GPIO_Type GPIO[9];
  __ne__ uint8_t RESERVED0[0x3000 - sizeof(STM_GPIO_Type) * 9];
  STM_CRC_Type CRC;
  __ne__ uint8_t RESERVED1[0x800 - sizeof(STM_CRC_Type)];
  STM_RCC_Type RCC;
  __ne__ uint8_t RESERVED2[0x400 - sizeof(STM_RCC_Type)];
  STM_FLASH_Type FLASH;
  __ne__ uint8_t RESERVED3[0x400 - sizeof(STM_FLASH_Type)];
  __rw__ uint32_t BKP_SRAM[1024];
  __ne__ uint8_t RESERVED4[0x2000 - sizeof(__rw__ uint32_t) * 1024];
  STM_DMA_Type DMA1;
  __ne__ uint8_t RESERVED5[0x400 - sizeof(STM_DMA_Type)];
  STM_DMA_Type DMA2;
  __ne__ uint8_t RESERVED6[0x1C00 - sizeof(STM_DMA_Type)];
  STM_ETHERNET_Type ETHERNET;
  __ne__ uint8_t RESERVED7[0x18000 - sizeof(STM_ETHERNET_Type)];
  STM_USB_OTG_Type USB_OTG_HS;
} AHB1_DOMAIN_Type;

typedef struct
{
  STM_USB_OTG_Type USB_OTG_FS;
  __ne__ uint8_t RESERVED0[0x50000 - sizeof(STM_USB_OTG_Type)];
  STM_DCMI_Type DCMI;
  __ne__ uint8_t RESERVED1[0x10000 - sizeof(STM_DCMI_Type)];
  STM_CRYP_Type CRYP;
  __ne__ uint8_t RESERVED2[0x400 - sizeof(STM_CRYP_Type)];
  STM_HASH_Type HASH;
  __ne__ uint8_t RESERVED3[0x400 - sizeof(STM_HASH_Type)];
  STM_RNG_Type RNG;
} AHB2_DOMAIN_Type;

typedef struct
{
  STM_FSMC_Type FSMC;
} AHB3_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern AHB1_DOMAIN_Type AHB1_DOMAIN;
extern AHB2_DOMAIN_Type AHB2_DOMAIN;
extern AHB3_DOMAIN_Type AHB3_DOMAIN;
extern APB1_DOMAIN_Type APB1_DOMAIN;
extern APB2_DOMAIN_Type APB2_DOMAIN;
/*----------------------------------------------------------------------------*/
//#define STM_ETHERNET_MAC_BASE   (STM_ETHERNET_BASE)
//#define STM_ETHERNET_MMC_BASE   (STM_ETHERNET_BASE + 0x0100)
//#define STM_ETHERNET_PTP_BASE   (STM_ETHERNET_BASE + 0x0700)
//#define STM_ETHERNET_DMA_BASE   (STM_ETHERNET_BASE + 0x1000)

#define STM_TIM2          (&APB1_DOMAIN.TIM2)
#define STM_TIM3          (&APB1_DOMAIN.TIM3)
#define STM_TIM4          (&APB1_DOMAIN.TIM4)
#define STM_TIM5          (&APB1_DOMAIN.TIM5)
#define STM_TIM6          (&APB1_DOMAIN.TIM6)
#define STM_TIM7          (&APB1_DOMAIN.TIM7)
#define STM_TIM12         (&APB1_DOMAIN.TIM12)
#define STM_TIM13         (&APB1_DOMAIN.TIM13)
#define STM_TIM14         (&APB1_DOMAIN.TIM14)
#define STM_RTC           (&APB1_DOMAIN.RTC)
#define STM_WWDG          (&APB1_DOMAIN.WWDG)
#define STM_IWDG          (&APB1_DOMAIN.IWDG)
#define STM_SPI2EXT       (&APB1_DOMAIN.SPI2EXT)
#define STM_SPI2          (&APB1_DOMAIN.SPI2)
#define STM_SPI3          (&APB1_DOMAIN.SPI3)
#define STM_SPI3EXT       (&APB1_DOMAIN.SPI3EXT)
#define STM_USART2        (&APB1_DOMAIN.USART2)
#define STM_USART3        (&APB1_DOMAIN.USART3)
#define STM_UART4         (&APB1_DOMAIN.UART4)
#define STM_UART5         (&APB1_DOMAIN.UART5)
#define STM_I2C1          (&APB1_DOMAIN.I2C1)
#define STM_I2C2          (&APB1_DOMAIN.I2C2)
#define STM_I2C3          (&APB1_DOMAIN.I2C3)
#define STM_CAN1          (&APB1_DOMAIN.CAN1)
#define STM_CAN2          (&APB1_DOMAIN.CAN2)
#define STM_PWR           (&APB1_DOMAIN.PWR)
#define STM_DAC           (&APB1_DOMAIN.DAC)

#define STM_TIM1          (&APB2_DOMAIN.TIM1)
#define STM_TIM8          (&APB2_DOMAIN.TIM8)
#define STM_USART1        (&APB2_DOMAIN.USART1)
#define STM_USART6        (&APB2_DOMAIN.USART6)
#define STM_ADC1          (&APB2_DOMAIN.ADC1)
#define STM_ADC2          (&APB2_DOMAIN.ADC2)
#define STM_ADC3          (&APB2_DOMAIN.ADC3)
#define STM_ADC_COMMON    (&APB2_DOMAIN.ADC_COMMON)
#define STM_SDIO          (&APB2_DOMAIN.SDIO)
#define STM_SPI1          (&APB2_DOMAIN.SPI1)
#define STM_SYSCFG        (&APB2_DOMAIN.SYSCFG)
#define STM_EXTI          (&APB2_DOMAIN.EXTI)
#define STM_TIM9          (&APB2_DOMAIN.TIM9)
#define STM_TIM10         (&APB2_DOMAIN.TIM10)
#define STM_TIM11         (&APB2_DOMAIN.TIM11)

#define STM_GPIO          (AHB1_DOMAIN.GPIO)
#define STM_GPIOA         (&AHB1_DOMAIN.GPIO[0])
#define STM_GPIOB         (&AHB1_DOMAIN.GPIO[1])
#define STM_GPIOC         (&AHB1_DOMAIN.GPIO[2])
#define STM_GPIOD         (&AHB1_DOMAIN.GPIO[3])
#define STM_GPIOE         (&AHB1_DOMAIN.GPIO[4])
#define STM_GPIOF         (&AHB1_DOMAIN.GPIO[5])
#define STM_GPIOG         (&AHB1_DOMAIN.GPIO[6])
#define STM_GPIOH         (&AHB1_DOMAIN.GPIO[7])
#define STM_GPIOI         (&AHB1_DOMAIN.GPIO[8])
#define STM_CRC           (&AHB1_DOMAIN.CRC)
#define STM_RCC           (&AHB1_DOMAIN.RCC)
#define STM_FLASH         (&AHB1_DOMAIN.FLASH)
#define STM_BKP_SRAM      (AHB1_DOMAIN.BKP_SRAM)
#define STM_DMA1          (&AHB1_DOMAIN.DMA1)
#define STM_DMA2          (&AHB1_DOMAIN.DMA2)
#define STM_ETHERNET      (&AHB1_DOMAIN.ETHERNET)
#define STM_USB_OTG_HS    (&AHB1_DOMAIN.USB_OTG_HS)

#define STM_USB_OTG_FS    (&AHB2_DOMAIN.USB_OTG_FS)
#define STM_DCMI          (&AHB2_DOMAIN.DCMI)
#define STM_CRYP          (&AHB2_DOMAIN.CRYP)
#define STM_HASH          (&AHB2_DOMAIN.HASH)
#define STM_RNG           (&AHB2_DOMAIN.RNG)

#define STM_FSMC          (&AHB3_DOMAIN.FSMC)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F4XX_PLATFORM_DEFS_H_ */
