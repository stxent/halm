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
  __rw__ uint32_t RF0R;
  __rw__ uint32_t RF1R;
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
  STM_CAN_FILTER_Type FILTERS[];
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
  STM_DMA_CHANNEL_Type CHANNELS[];
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
  __rw__ uint32_t CCR1;
  __rw__ uint32_t CCR2;
  __rw__ uint32_t CCR3;
  __rw__ uint32_t CCR4;
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
#define STM_OB_BASE             0x1FFFF800UL
#define STM_APB1_BASE           0x40000000UL
#define STM_APB2_BASE           0x40010000UL
#define STM_AHB_BASE            0x40020000UL
#define STM_FSMC_BASE           0xA0000000UL

#define STM_TIM2_BASE           (STM_APB1_BASE + 0x0000)
#define STM_TIM3_BASE           (STM_APB1_BASE + 0x0400)
#define STM_TIM4_BASE           (STM_APB1_BASE + 0x0800)
#define STM_TIM5_BASE           (STM_APB1_BASE + 0x0C00)
#define STM_TIM6_BASE           (STM_APB1_BASE + 0x1000)
#define STM_TIM7_BASE           (STM_APB1_BASE + 0x1400)
#define STM_TIM12_BASE          (STM_APB1_BASE + 0x1800)
#define STM_TIM13_BASE          (STM_APB1_BASE + 0x1C00)
#define STM_TIM14_BASE          (STM_APB1_BASE + 0x2000)
#define STM_RTC_BASE            (STM_APB1_BASE + 0x2800)
#define STM_WWDG_BASE           (STM_APB1_BASE + 0x2C00)
#define STM_IWDG_BASE           (STM_APB1_BASE + 0x3000)
#define STM_SPI2_BASE           (STM_APB1_BASE + 0x3800)
#define STM_SPI3_BASE           (STM_APB1_BASE + 0x3C00)
#define STM_USART2_BASE         (STM_APB1_BASE + 0x4400)
#define STM_USART3_BASE         (STM_APB1_BASE + 0x4800)
#define STM_UART4_BASE          (STM_APB1_BASE + 0x4C00)
#define STM_UART5_BASE          (STM_APB1_BASE + 0x5000)
#define STM_I2C1_BASE           (STM_APB1_BASE + 0x5400)
#define STM_I2C2_BASE           (STM_APB1_BASE + 0x5800)
#define STM_USB_BASE            (STM_APB1_BASE + 0x5C00)
#define STM_CAN_USB_SRAM_BASE   (STM_APB1_BASE + 0x6000)
#define STM_CAN1_BASE           (STM_APB1_BASE + 0x6400)
#define STM_CAN2_BASE           (STM_APB1_BASE + 0x6800)
#define STM_PWR_BASE            (STM_APB1_BASE + 0x7000)
#define STM_DAC_BASE            (STM_APB1_BASE + 0x7400)

#define STM_AFIO_BASE           (STM_APB2_BASE + 0x0000)
#define STM_EXTI_BASE           (STM_APB2_BASE + 0x0400)
#define STM_GPIOA_BASE          (STM_APB2_BASE + 0x0800)
#define STM_GPIOB_BASE          (STM_APB2_BASE + 0x0C00)
#define STM_GPIOC_BASE          (STM_APB2_BASE + 0x1000)
#define STM_GPIOD_BASE          (STM_APB2_BASE + 0x1400)
#define STM_GPIOE_BASE          (STM_APB2_BASE + 0x1800)
#define STM_GPIOF_BASE          (STM_APB2_BASE + 0x1C00)
#define STM_GPIOG_BASE          (STM_APB2_BASE + 0x2000)
#define STM_ADC1_BASE           (STM_APB2_BASE + 0x2400)
#define STM_ADC2_BASE           (STM_APB2_BASE + 0x2800)
#define STM_TIM1_BASE           (STM_APB2_BASE + 0x2C00)
#define STM_SPI1_BASE           (STM_APB2_BASE + 0x3000)
#define STM_TIM8_BASE           (STM_APB2_BASE + 0x3400)
#define STM_USART1_BASE         (STM_APB2_BASE + 0x3800)
#define STM_ADC3_BASE           (STM_APB2_BASE + 0x3C00)
#define STM_TIM15_BASE          (STM_APB2_BASE + 0x4000)
#define STM_TIM16_BASE          (STM_APB2_BASE + 0x4400)
#define STM_TIM17_BASE          (STM_APB2_BASE + 0x4800)
#define STM_TIM9_BASE           (STM_APB2_BASE + 0x4C00)
#define STM_TIM10_BASE          (STM_APB2_BASE + 0x5000)
#define STM_TIM11_BASE          (STM_APB2_BASE + 0x5400)

#define STM_SDIO_BASE           (STM_AHB_BASE - 0x2000)
#define STM_DMA1_BASE           (STM_AHB_BASE + 0x0000)
#define STM_DMA2_BASE           (STM_AHB_BASE + 0x0400)
#define STM_RCC_BASE            (STM_AHB_BASE + 0x1000)
#define STM_FLASH_BASE          (STM_AHB_BASE + 0x2000)
#define STM_CRC_BASE            (STM_AHB_BASE + 0x3000)
#define STM_ETHERNET_BASE       (STM_AHB_BASE + 0x8000)

#define STM_ETHERNET_MAC_BASE   (STM_ETHERNET_BASE)
#define STM_ETHERNET_MMC_BASE   (STM_ETHERNET_BASE + 0x0100)
#define STM_ETHERNET_PTP_BASE   (STM_ETHERNET_BASE + 0x0700)
#define STM_ETHERNET_DMA_BASE   (STM_ETHERNET_BASE + 0x1000)

#define STM_TIM2                ((STM_TIM_Type *)STM_TIM2_BASE)
#define STM_TIM3                ((STM_TIM_Type *)STM_TIM3_BASE)
#define STM_TIM4                ((STM_TIM_Type *)STM_TIM4_BASE)
#define STM_TIM5                ((STM_TIM_Type *)STM_TIM5_BASE)
#define STM_TIM6                ((STM_TIM_Type *)STM_TIM6_BASE)
#define STM_TIM7                ((STM_TIM_Type *)STM_TIM7_BASE)
#define STM_TIM12               ((STM_TIM_Type *)STM_TIM12_BASE)
#define STM_TIM13               ((STM_TIM_Type *)STM_TIM13_BASE)
#define STM_TIM14               ((STM_TIM_Type *)STM_TIM14_BASE)
#define STM_RTC                 ((STM_RTC_Type *)STM_RTC_BASE)
#define STM_WWDG                ((STM_WWDG_Type *)STM_WWDG_BASE)
#define STM_IWDG                ((STM_IWDG_Type *)STM_IWDG_BASE)
#define STM_SPI2                ((STM_SPI_Type *)STM_SPI2_BASE)
#define STM_SPI3                ((STM_SPI_Type *)STM_SPI3_BASE)
#define STM_USART2              ((STM_USART_Type *)STM_USART2_BASE)
#define STM_USART3              ((STM_USART_Type *)STM_USART3_BASE)
#define STM_UART4               ((STM_USART_Type *)STM_UART4_BASE)
#define STM_UART5               ((STM_USART_Type *)STM_UART5_BASE)
#define STM_I2C1                ((STM_I2C_Type *)STM_I2C1_BASE)
#define STM_I2C2                ((STM_I2C_Type *)STM_I2C2_BASE)
#define STM_USB                 ((STM_USB_Type *)STM_USB_BASE)
#define STM_CAN1                ((STM_CAN_Type *)STM_CAN1_BASE)
#define STM_CAN2                ((STM_CAN_Type *)STM_CAN2_BASE)
#define STM_BKP                 ((STM_BKP_Type *)STM_BKP_BASE)
#define STM_PWR                 ((STM_PWR_Type *)STM_PWR_BASE)
#define STM_DAC                 ((STM_DAC_Type *)STM_DAC_BASE)
#define STM_AFIO                ((STM_AFIO_Type *)STM_AFIO_BASE)
#define STM_EXTI                ((STM_EXTI_Type *)STM_EXTI_BASE)
#define STM_GPIOA               ((STM_GPIO_Type *)STM_GPIOA_BASE)
#define STM_GPIOB               ((STM_GPIO_Type *)STM_GPIOB_BASE)
#define STM_GPIOC               ((STM_GPIO_Type *)STM_GPIOC_BASE)
#define STM_GPIOD               ((STM_GPIO_Type *)STM_GPIOD_BASE)
#define STM_GPIOE               ((STM_GPIO_Type *)STM_GPIOE_BASE)
#define STM_GPIOF               ((STM_GPIO_Type *)STM_GPIOF_BASE)
#define STM_GPIOG               ((STM_GPIO_Type *)STM_GPIOG_BASE)
#define STM_ADC1                ((STM_ADC_Type *)STM_ADC1_BASE)
#define STM_ADC2                ((STM_ADC_Type *)STM_ADC2_BASE)
#define STM_TIM1                ((STM_TIM_Type *)STM_TIM1_BASE)
#define STM_SPI1                ((STM_SPI_Type *)STM_SPI1_BASE)
#define STM_TIM8                ((STM_TIM_Type *)STM_TIM8_BASE)
#define STM_USART1              ((STM_USART_Type *)STM_USART1_BASE)
#define STM_ADC3                ((STM_ADC_Type *)STM_ADC3_BASE)
#define STM_TIM15               ((STM_TIM_Type *)STM_TIM15_BASE)
#define STM_TIM16               ((STM_TIM_Type *)STM_TIM16_BASE)
#define STM_TIM17               ((STM_TIM_Type *)STM_TIM17_BASE)
#define STM_TIM9                ((STM_TIM_Type *)STM_TIM9_BASE)
#define STM_TIM10               ((STM_TIM_Type *)STM_TIM10_BASE)
#define STM_TIM11               ((STM_TIM_Type *)STM_TIM11_BASE)
#define STM_SDIO                ((STM_SDIO_Type *)STM_SDIO_BASE)
#define STM_DMA1                ((STM_DMA_Type *)STM_DMA1_BASE)
#define STM_DMA2                ((STM_DMA_Type *)STM_DMA2_BASE)
#define STM_RCC                 ((STM_RCC_Type *)STM_RCC_BASE)
#define STM_CRC                 ((STM_CRC_Type *)STM_CRC_BASE)
#define STM_FLASH               ((STM_FLASH_Type *)STM_FLASH_BASE)
#define STM_OB                  ((STM_OB_Type *)STM_OB_BASE)
#define STM_ETHERNET            ((STM_ETHERNET_Type *)STM_ETHERNET_BASE)
#define STM_FSMC                ((STM_FSMC_Type *)STM_FSMC_BASE)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM_STM32F1XX_PLATFORM_DEFS_H_ */
