/*
 * halm/platform/stm/stm32f1xx/platform_defs.h
 * Based on original from ST
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_STM_STM32F1XX_PLATFORM_DEFS_H_
#define HALM_PLATFORM_STM_STM32F1XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#define NVIC_PRIORITY_SIZE 4
/*------------------Analog to Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t SR;
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t SMPR1;
  __rw__ uint32_t SMPR2;
  __rw__ uint32_t JOFR1;
  __rw__ uint32_t JOFR2;
  __rw__ uint32_t JOFR3;
  __rw__ uint32_t JOFR4;
  __rw__ uint32_t HTR;
  __rw__ uint32_t LTR;
  __rw__ uint32_t SQR1;
  __rw__ uint32_t SQR2;
  __rw__ uint32_t SQR3;
  __rw__ uint32_t JSQR;
  __rw__ uint32_t JDR1;
  __rw__ uint32_t JDR2;
  __rw__ uint32_t JDR3;
  __rw__ uint32_t JDR4;
  __rw__ uint32_t DR;
} STM_ADC_Type;
/*------------------Backup registers------------------------------------------*/
typedef struct
{
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t DRL[10]; /* Lower part of registers, DR1..DR10 */
  __rw__ uint32_t RTCCR;
  __rw__ uint32_t CR;
  __rw__ uint32_t CSR;
  __ne__ uint32_t RESERVED1[2];
  __rw__ uint32_t DRH[32]; /* Higher part of registers, DR11..DR42 */
} STM_BKP_Type;
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
  __rw__ uint32_t SWTRIGR;
  __rw__ uint32_t DHR12R1;
  __rw__ uint32_t DHR12L1;
  __rw__ uint32_t DHR8R1;
  __rw__ uint32_t DHR12R2;
  __rw__ uint32_t DHR12L2;
  __rw__ uint32_t DHR8R2;
  __rw__ uint32_t DHR12RD;
  __rw__ uint32_t DHR12LD;
  __rw__ uint32_t DHR8RD;
  __rw__ uint32_t DOR1;
  __rw__ uint32_t DOR2;
  __rw__ uint32_t SR; /* For LD, MD and HD parts */
} STM_DAC_Type;
/*------------------DMA Controller--------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CCR;
  __rw__ uint32_t CNDTR;
  __rw__ uint32_t CPAR;
  __rw__ uint32_t CMAR;
  __ne__ uint32_t RESERVED;
} STM_DMA_CHANNEL_Type;

typedef struct
{
  __rw__ uint32_t ISR;
  __rw__ uint32_t IFCR;

  /* Offset 0x08: channel registers, 7 for DMA1 and 5 for DMA2 */
  STM_DMA_CHANNEL_Type CHANNELS[7];
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
  __ne__ uint32_t RESERVED8[567];
  __rw__ uint32_t DMABMR;
  __rw__ uint32_t DMATPDR;
  __rw__ uint32_t DMARPDR;
  __rw__ uint32_t DMARDLAR;
  __rw__ uint32_t DMATDLAR;
  __rw__ uint32_t DMASR;
  __rw__ uint32_t DMAOMR;
  __rw__ uint32_t DMAIER;
  __rw__ uint32_t DMAMFBOCR;
  __ne__ uint32_t RESERVED9[9];
  __rw__ uint32_t DMACHTDR;
  __rw__ uint32_t DMACHRDR;
  __rw__ uint32_t DMACHTBAR;
  __rw__ uint32_t DMACHRBAR;
} STM_ETHERNET_Type;
/*------------------USB full-speed device-------------------------------------*/
typedef struct
{
  union
  {
    struct
    {
      __rw__ uint32_t EP0R;
      __rw__ uint32_t EP1R;
      __rw__ uint32_t EP2R;
      __rw__ uint32_t EP3R;
      __rw__ uint32_t EP4R;
      __rw__ uint32_t EP5R;
      __rw__ uint32_t EP6R;
      __rw__ uint32_t EP7R;
    };
    __rw__ uint32_t EPR[8];
  };

  __ne__ uint32_t RESERVED0[8];

  /* Offset 0x40 */
  __rw__ uint32_t CNTR;
  __rw__ uint32_t ISTR;
  __rw__ uint32_t FNR;
  __rw__ uint32_t DADDR;
  __rw__ uint32_t BTABLE;
} STM_USB_Type;
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
  __rw__ uint32_t AR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t OBR;
  __rw__ uint32_t WRPR;

  /* Offset 0x24: for XL parts */
  __ne__ uint32_t RESERVED1[8];
  __rw__ uint32_t KEYR2;
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t SR2;
  __rw__ uint32_t CR2;
  __rw__ uint32_t AR2;
} STM_FLASH_Type;
/*------------------Option Bytes----------------------------------------------*/
typedef struct
{
  __rw__ uint16_t RDP;
  __rw__ uint16_t USER;
  __rw__ uint16_t Data0;
  __rw__ uint16_t Data1;
  __rw__ uint16_t WRP0;
  __rw__ uint16_t WRP1;
  __rw__ uint16_t WRP2;
  __rw__ uint16_t WRP3;
} STM_OB_Type;
/*------------------Flexible Static Memory Controller-------------------------*/
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
  union
  {
    struct
    {
      __rw__ uint32_t CRL;
      __rw__ uint32_t CRH;
    };

    __rw__ uint32_t CR[2];
  };
  __rw__ uint32_t IDR;
  __rw__ uint32_t ODR;
  __rw__ uint32_t BSRR;
  __rw__ uint32_t BRR;
  __rw__ uint32_t LCKR;
  __ne__ uint32_t RESERVED0[249];
} STM_GPIO_Type;
/*------------------Alternate Function Input/Output---------------------------*/
typedef struct
{
  __rw__ uint32_t EVCR;
  __rw__ uint32_t MAPR;
  __rw__ uint32_t EXTICR[4];
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t MAPR2;
} STM_AFIO_Type;
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
  __rw__ uint32_t KR;
  __rw__ uint32_t PR;
  __rw__ uint32_t RLR;
  __rw__ uint32_t SR;
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
  __rw__ uint32_t CFGR;
  __rw__ uint32_t CIR;
  __rw__ uint32_t APB2RSTR;
  __rw__ uint32_t APB1RSTR;
  __rw__ uint32_t AHBENR;
  __rw__ uint32_t APB2ENR;
  __rw__ uint32_t APB1ENR;
  __rw__ uint32_t BDCR;
  __rw__ uint32_t CSR;
  __rw__ uint32_t AHBRSTR; /* For CL parts */
  __rw__ uint32_t CFGR2; /* For CL, LD, MD, HD parts */
} STM_RCC_Type;
/*------------------Real-Time Clock-------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CRH;
  __rw__ uint32_t CRL;
  __rw__ uint32_t PRLH;
  __rw__ uint32_t PRLL;
  __rw__ uint32_t DIVH;
  __rw__ uint32_t DIVL;
  __rw__ uint32_t CNTH;
  __rw__ uint32_t CNTL;
  __rw__ uint32_t ALRH;
  __rw__ uint32_t ALRL;
} STM_RTC_Type;
/*------------------SD/MMC card interface-------------------------------------*/
typedef struct
{
  __rw__ uint32_t POWER;
  __rw__ uint32_t CLKCR;
  __rw__ uint32_t ARG;
  __rw__ uint32_t CMD;
  __ro__ uint32_t RESPCMD;
  __ro__ uint32_t RESP1;
  __ro__ uint32_t RESP2;
  __ro__ uint32_t RESP3;
  __ro__ uint32_t RESP4;
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
  __rw__ uint32_t RXCRCR;
  __rw__ uint32_t TXCRCR;
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
  __rw__ uint32_t EGR;
  __rw__ uint32_t CCMR1;
  __rw__ uint32_t CCMR2;
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
  __ne__ uint8_t RESERVED11[0x800 - sizeof(STM_IWDG_Type)];
  STM_SPI_Type SPI2;
  __ne__ uint8_t RESERVED12[0x400 - sizeof(STM_SPI_Type)];
  STM_SPI_Type SPI3;
  __ne__ uint8_t RESERVED13[0x800 - sizeof(STM_SPI_Type)];
  STM_USART_Type USART2;
  __ne__ uint8_t RESERVED14[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART3;
  __ne__ uint8_t RESERVED15[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type UART4;
  __ne__ uint8_t RESERVED16[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type UART5;
  __ne__ uint8_t RESERVED17[0x400 - sizeof(STM_USART_Type)];
  STM_I2C_Type I2C1;
  __ne__ uint8_t RESERVED18[0x400 - sizeof(STM_I2C_Type)];
  STM_I2C_Type I2C2;
  __ne__ uint8_t RESERVED19[0x400 - sizeof(STM_I2C_Type)];
  STM_USB_Type USB;
  __ne__ uint8_t RESERVED20[0x400 - sizeof(STM_USB_Type)];
  __rw__ uint16_t CAN_USB_SRAM[512];
  STM_CAN_Type CAN1;
  __ne__ uint8_t RESERVED21[0x400 - sizeof(STM_CAN_Type)];
  STM_CAN_Type CAN2;
  __ne__ uint8_t RESERVED22[0x400 - sizeof(STM_CAN_Type)];
  STM_BKP_Type BKP;
  __ne__ uint8_t RESERVED23[0x400 - sizeof(STM_BKP_Type)];
  STM_PWR_Type PWR;
  __ne__ uint8_t RESERVED24[0x400 - sizeof(STM_PWR_Type)];
  STM_DAC_Type DAC;
} APB1_DOMAIN_Type;

typedef struct
{
  STM_AFIO_Type AFIO;
  __ne__ uint8_t RESERVED0[0x400 - sizeof(STM_AFIO_Type)];
  STM_EXTI_Type EXTI;
  __ne__ uint8_t RESERVED1[0x400 - sizeof(STM_EXTI_Type)];
  STM_GPIO_Type GPIO[7];
  STM_ADC_Type ADC1;
  __ne__ uint8_t RESERVED2[0x400 - sizeof(STM_ADC_Type)];
  STM_ADC_Type ADC2;
  __ne__ uint8_t RESERVED3[0x400 - sizeof(STM_ADC_Type)];
  STM_TIM_Type TIM1;
  __ne__ uint8_t RESERVED4[0x400 - sizeof(STM_TIM_Type)];
  STM_SPI_Type SPI1;
  __ne__ uint8_t RESERVED5[0x400 - sizeof(STM_SPI_Type)];
  STM_TIM_Type TIM8;
  __ne__ uint8_t RESERVED6[0x400 - sizeof(STM_TIM_Type)];
  STM_USART_Type USART1;
  __ne__ uint8_t RESERVED7[0x400 - sizeof(STM_USART_Type)];
  STM_ADC_Type ADC3;
  __ne__ uint8_t RESERVED8[0x400 - sizeof(STM_ADC_Type)];
  STM_TIM_Type TIM15;
  __ne__ uint8_t RESERVED9[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM16;
  __ne__ uint8_t RESERVED10[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM17;
  __ne__ uint8_t RESERVED11[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM9;
  __ne__ uint8_t RESERVED12[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM10;
  __ne__ uint8_t RESERVED13[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM11;
} APB2_DOMAIN_Type;

typedef struct
{
  STM_SDIO_Type SDIO;
  __ne__ uint8_t RESERVED0[0x8000 - sizeof(STM_SDIO_Type)];
  STM_DMA_Type DMA1;
  __ne__ uint8_t RESERVED1[0x400 - sizeof(STM_DMA_Type)];
  STM_DMA_Type DMA2;
  __ne__ uint8_t RESERVED2[0xC00 - sizeof(STM_DMA_Type)];
  STM_RCC_Type RCC;
  __ne__ uint8_t RESERVED3[0x1000 - sizeof(STM_RCC_Type)];
  STM_FLASH_Type FLASH;
  __ne__ uint8_t RESERVED4[0x1000 - sizeof(STM_FLASH_Type)];
  STM_CRC_Type CRC;
  __ne__ uint8_t RESERVED5[0x5000 - sizeof(STM_CRC_Type)];
  STM_ETHERNET_Type ETHERNET;
} AHB_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern APB1_DOMAIN_Type APB1_DOMAIN;
extern APB2_DOMAIN_Type APB2_DOMAIN;
extern AHB_DOMAIN_Type  AHB_DOMAIN;
extern STM_OB_Type      OB_DOMAIN;
extern STM_FSMC_Type    FSMC_DOMAIN;
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
#define STM_SPI2          (&APB1_DOMAIN.SPI2)
#define STM_SPI3          (&APB1_DOMAIN.SPI3)
#define STM_USART2        (&APB1_DOMAIN.USART2)
#define STM_USART3        (&APB1_DOMAIN.USART3)
#define STM_UART4         (&APB1_DOMAIN.UART4)
#define STM_UART5         (&APB1_DOMAIN.UART5)
#define STM_I2C1          (&APB1_DOMAIN.I2C1)
#define STM_I2C2          (&APB1_DOMAIN.I2C2)
#define STM_CAN_USB_SRAM  (APB1_DOMAIN.CAN_USB_SRAM)
#define STM_USB           (&APB1_DOMAIN.USB)
#define STM_CAN1          (&APB1_DOMAIN.CAN1)
#define STM_CAN2          (&APB1_DOMAIN.CAN2)
#define STM_BKP           (&APB1_DOMAIN.BKP)
#define STM_PWR           (&APB1_DOMAIN.PWR)
#define STM_DAC           (&APB1_DOMAIN.DAC)

#define STM_AFIO          (&APB2_DOMAIN.AFIO)
#define STM_EXTI          (&APB2_DOMAIN.EXTI)
#define STM_GPIO          (APB2_DOMAIN.GPIO)
#define STM_GPIOA         (&APB2_DOMAIN.GPIO[0])
#define STM_GPIOB         (&APB2_DOMAIN.GPIO[1])
#define STM_GPIOC         (&APB2_DOMAIN.GPIO[2])
#define STM_GPIOD         (&APB2_DOMAIN.GPIO[3])
#define STM_GPIOE         (&APB2_DOMAIN.GPIO[4])
#define STM_GPIOF         (&APB2_DOMAIN.GPIO[5])
#define STM_GPIOG         (&APB2_DOMAIN.GPIO[6])
#define STM_ADC1          (&APB2_DOMAIN.ADC1)
#define STM_ADC2          (&APB2_DOMAIN.ADC2)
#define STM_TIM1          (&APB2_DOMAIN.TIM1)
#define STM_SPI1          (&APB2_DOMAIN.SPI1)
#define STM_TIM8          (&APB2_DOMAIN.TIM8)
#define STM_USART1        (&APB2_DOMAIN.USART1)
#define STM_ADC3          (&APB2_DOMAIN.ADC3)
#define STM_TIM15         (&APB2_DOMAIN.TIM15)
#define STM_TIM16         (&APB2_DOMAIN.TIM16)
#define STM_TIM17         (&APB2_DOMAIN.TIM17)
#define STM_TIM9          (&APB2_DOMAIN.TIM9)
#define STM_TIM10         (&APB2_DOMAIN.TIM10)
#define STM_TIM11         (&APB2_DOMAIN.TIM11)

#define STM_SDIO          (&AHB_DOMAIN.SDIO)
#define STM_DMA1          (&AHB_DOMAIN.DMA1)
#define STM_DMA2          (&AHB_DOMAIN.DMA2)
#define STM_RCC           (&AHB_DOMAIN.RCC)
#define STM_CRC           (&AHB_DOMAIN.CRC)
#define STM_FLASH         (&AHB_DOMAIN.FLASH)
#define STM_ETHERNET      (&AHB_DOMAIN.ETHERNET)

#define STM_OB            (&OB_DOMAIN)
#define STM_FSMC          (&FSMC_DOMAIN)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_PLATFORM_DEFS_H_ */
