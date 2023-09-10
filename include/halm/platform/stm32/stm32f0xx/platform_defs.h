/*
 * halm/platform/stm32/stm32f0xx/platform_defs.h
 * Based on original from ST
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_PLATFORM_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_STM32F0XX_PLATFORM_DEFS_H_
#define HALM_PLATFORM_STM32_STM32F0XX_PLATFORM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <stdint.h>
/*------------------Analog to Digital Converter-------------------------------*/
typedef struct
{
  __rw__ uint32_t ISR;
  __rw__ uint32_t IER;
  __rw__ uint32_t CR;
  __rw__ uint32_t CFGR1;
  __rw__ uint32_t CFGR2;
  __rw__ uint32_t SMPR;
  __ne__ uint32_t RESERVED0[2];
  __rw__ uint32_t TR;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t CHSELR;
  __ne__ uint32_t RESERVED2[5];
  __ro__ uint32_t DR;
  __ne__ uint32_t RESERVED3[176];

  /* Offset 0x304 */
  __rw__ uint32_t CCR;
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
/*------------------HDMI-CEC--------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t CFGR;
  __wo__ uint32_t TXDR;
  __ro__ uint32_t RXDR;
  __rw__ uint32_t ISR;
  __rw__ uint32_t IER;
} STM_CEC_Type;
/*------------------CRC calculation unit--------------------------------------*/
typedef struct
{
  __rw__ uint32_t DR;
  __rw__ uint32_t IDR;
  __rw__ uint32_t CR;
  __rw__ uint32_t INIT;
  __rw__ uint32_t POL;
} STM_CRC_Type;
/*------------------Clock Recovery System-------------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t CFGR;
  __ro__ uint32_t ISR;
  __rw__ uint32_t ICR;
} STM_CRS_Type;
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
/*------------------MCU Debug Component---------------------------------------*/
typedef struct
{
  __ro__ uint32_t IDCODE;
  __rw__ uint32_t CR;
  __rw__ uint32_t APB1FZ;
  __rw__ uint32_t APB2FZ;
} STM_DBGMCU_Type;
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
} STM_FLASH_Type;
/*------------------Option Bytes----------------------------------------------*/
typedef struct
{
  __rw__ uint16_t RDP;
  __rw__ uint16_t USER;
  __rw__ uint16_t DATA0;
  __rw__ uint16_t DATA1;
  __rw__ uint16_t WRP0;
  __rw__ uint16_t WRP1;
  __rw__ uint16_t WRP2;
  __rw__ uint16_t WRP3;
} STM_OB_Type;
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
  __rw__ uint32_t BRR;
  __ne__ uint32_t RESERVED0[245];
} STM_GPIO_Type;
/*------------------Inter-Integrated Circuit----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t OAR1;
  __rw__ uint32_t OAR2;
  __rw__ uint32_t TIMINGR;
  __rw__ uint32_t TIMEOUTR;
  __rw__ uint32_t ISR;
  __wo__ uint32_t ICR;
  __ro__ uint32_t PECR;
  __ro__ uint32_t RXDR;
  __rw__ uint32_t TXDR;
} STM_I2C_Type;
/*------------------Independent Watchdog--------------------------------------*/
typedef struct
{
  __wo__ uint32_t KR;
  __rw__ uint32_t PR;
  __rw__ uint32_t RLR;
  __ro__ uint32_t SR;
  __rw__ uint32_t WINR;
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
  __rw__ uint32_t AHBRSTR;
  __rw__ uint32_t CFGR2;
  __rw__ uint32_t CFGR3;
  __rw__ uint32_t CR2;
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
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t ALRMAR;
  __ne__ uint32_t RESERVED1;
  __wo__ uint32_t WPR;
  __ro__ uint32_t SSR;
  __wo__ uint32_t SHIFTR;
  __ro__ uint32_t TSTR;
  __ro__ uint32_t TSDR;
  __ro__ uint32_t TSSSR;
  __rw__ uint32_t CALR;
  __rw__ uint32_t TAFCR;
  __rw__ uint32_t ALRMASSR;
  __ne__ uint32_t RESERVED2[2];

  /* Offset 0x50 */
  union
  {
    struct
    {
      __rw__ uint32_t BKP0R;
      __rw__ uint32_t BKP1R;
      __rw__ uint32_t BKP2R;
      __rw__ uint32_t BKP3R;
      __rw__ uint32_t BKP4R;
    };
    __rw__ uint32_t BKPR[5];
  };
} STM_RTC_Type;
/*------------------Comparator------------------------------------------------*/
typedef struct
{
  __rw__ uint32_t CSR;
} STM_COMP_Type;
/*------------------System Configuration--------------------------------------*/
typedef struct
{
  __rw__ uint32_t CFGR1;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t EXTICR[4];
  __rw__ uint32_t CFGR2;

  STM_COMP_Type COMP;

  __ne__ uint32_t RESERVED1[24];
  __rw__ uint32_t ITLINE[32];
} STM_SYSCFG_Type;
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
/*------------------Touch Sensing Controller----------------------------------*/
typedef struct
{
  __rw__ uint32_t CR;
  __rw__ uint32_t IER;
  __rw__ uint32_t ICR;
  __ro__ uint32_t ISR;
  __rw__ uint32_t IOHCR;
  __ne__ uint32_t RESERVED0;
  __rw__ uint32_t IOASCR;
  __ne__ uint32_t RESERVED1;
  __rw__ uint32_t IOSCR;
  __ne__ uint32_t RESERVED2;
  __rw__ uint32_t IOCCR;
  __ne__ uint32_t RESERVED3;
  __rw__ uint32_t IOGCSR;
  __ro__ uint32_t IOGCR[8];
} STM_TSC_Type;
/*------------------Universal Synchronous Asynchronous Receiver Transmitter---*/
typedef struct
{
  __rw__ uint32_t CR1;
  __rw__ uint32_t CR2;
  __rw__ uint32_t CR3;
  __rw__ uint32_t BRR;
  __rw__ uint32_t GTPR;
  __rw__ uint32_t RTOR;
  __wo__ uint32_t RQR;
  __ro__ uint32_t ISR;
  __rw__ uint32_t ICR;
  __ro__ uint32_t RDR;
  __rw__ uint32_t TDR;
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
  __ne__ uint8_t RESERVED1[0xC00 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM6;
  __ne__ uint8_t RESERVED2[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM7;
  __ne__ uint8_t RESERVED3[0xC00 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM14;
  __ne__ uint8_t RESERVED4[0x800 - sizeof(STM_TIM_Type)];
  STM_RTC_Type RTC;
  __ne__ uint8_t RESERVED5[0x400 - sizeof(STM_RTC_Type)];
  STM_WWDG_Type WWDG;
  __ne__ uint8_t RESERVED6[0x400 - sizeof(STM_WWDG_Type)];
  STM_IWDG_Type IWDG;
  __ne__ uint8_t RESERVED7[0x800 - sizeof(STM_IWDG_Type)];
  STM_SPI_Type SPI2;
  __ne__ uint8_t RESERVED8[0xC00 - sizeof(STM_SPI_Type)];
  STM_USART_Type USART2;
  __ne__ uint8_t RESERVED9[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART3;
  __ne__ uint8_t RESERVED10[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART4;
  __ne__ uint8_t RESERVED11[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART5;
  __ne__ uint8_t RESERVED12[0x400 - sizeof(STM_USART_Type)];
  STM_I2C_Type I2C1;
  __ne__ uint8_t RESERVED13[0x400 - sizeof(STM_I2C_Type)];
  STM_I2C_Type I2C2;
  __ne__ uint8_t RESERVED14[0x400 - sizeof(STM_I2C_Type)];
  STM_USB_Type USB;
  __ne__ uint8_t RESERVED15[0x400 - sizeof(STM_USB_Type)];
  __rw__ uint32_t CAN_USB_SRAM[256];
  STM_CAN_Type CAN;
  __ne__ uint8_t RESERVED16[0x800 - sizeof(STM_CAN_Type)];
  STM_CRS_Type CRS;
  __ne__ uint8_t RESERVED17[0x400 - sizeof(STM_CRS_Type)];
  STM_PWR_Type PWR;
  __ne__ uint8_t RESERVED18[0x400 - sizeof(STM_PWR_Type)];
  STM_DAC_Type DAC;
  __ne__ uint8_t RESERVED19[0x400 - sizeof(STM_DAC_Type)];
  STM_CEC_Type CEC;
  __ne__ uint8_t RESERVED20[0x8800 - sizeof(STM_CEC_Type)];
  STM_SYSCFG_Type SYSCFG;
  __ne__ uint8_t RESERVED21[0x400 - sizeof(STM_SYSCFG_Type)];
  STM_EXTI_Type EXTI;
  __ne__ uint8_t RESERVED22[0x1000 - sizeof(STM_EXTI_Type)];
  STM_USART_Type USART6;
  __ne__ uint8_t RESERVED23[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART7;
  __ne__ uint8_t RESERVED24[0x400 - sizeof(STM_USART_Type)];
  STM_USART_Type USART8;
  __ne__ uint8_t RESERVED25[0x800 - sizeof(STM_USART_Type)];
  STM_ADC_Type ADC;
  __ne__ uint8_t RESERVED26[0x800 - sizeof(STM_ADC_Type)];
  STM_TIM_Type TIM1;
  __ne__ uint8_t RESERVED27[0x400 - sizeof(STM_TIM_Type)];
  STM_SPI_Type SPI1;
  __ne__ uint8_t RESERVED28[0x800 - sizeof(STM_SPI_Type)];
  STM_USART_Type USART1;
  __ne__ uint8_t RESERVED29[0x800 - sizeof(STM_USART_Type)];
  STM_TIM_Type TIM15;
  __ne__ uint8_t RESERVED30[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM16;
  __ne__ uint8_t RESERVED31[0x400 - sizeof(STM_TIM_Type)];
  STM_TIM_Type TIM17;
  __ne__ uint8_t RESERVED32[0x1000 - sizeof(STM_TIM_Type)];
  STM_DBGMCU_Type DBGMCU;
} APB_DOMAIN_Type;

typedef struct
{
  STM_DMA_Type DMA1;
  __ne__ uint8_t RESERVED0[0x400 - sizeof(STM_DMA_Type)];
  STM_DMA_Type DMA2;
  __ne__ uint8_t RESERVED1[0xC00 - sizeof(STM_DMA_Type)];
  STM_RCC_Type RCC;
  __ne__ uint8_t RESERVED2[0x1000 - sizeof(STM_RCC_Type)];
  STM_FLASH_Type FLASH;
  __ne__ uint8_t RESERVED3[0x1000 - sizeof(STM_FLASH_Type)];
  STM_CRC_Type CRC;
  __ne__ uint8_t RESERVED4[0x1000 - sizeof(STM_CRC_Type)];
  STM_TSC_Type TSC;
} AHB1_DOMAIN_Type;

typedef struct
{
  STM_GPIO_Type GPIO[6];
} AHB2_DOMAIN_Type;
/*----------------------------------------------------------------------------*/
extern AHB1_DOMAIN_Type AHB1_DOMAIN;
extern AHB2_DOMAIN_Type AHB2_DOMAIN;
extern APB_DOMAIN_Type  APB_DOMAIN;
/*----------------------------------------------------------------------------*/
#define STM_TIM2          (&APB_DOMAIN.TIM2)
#define STM_TIM3          (&APB_DOMAIN.TIM3)
#define STM_TIM6          (&APB_DOMAIN.TIM6)
#define STM_TIM7          (&APB_DOMAIN.TIM7)
#define STM_TIM14         (&APB_DOMAIN.TIM14)
#define STM_RTC           (&APB_DOMAIN.RTC)
#define STM_WWDG          (&APB_DOMAIN.WWDG)
#define STM_IWDG          (&APB_DOMAIN.IWDG)
#define STM_SPI2          (&APB_DOMAIN.SPI2)
#define STM_USART2        (&APB_DOMAIN.USART2)
#define STM_USART3        (&APB_DOMAIN.USART3)
#define STM_USART4        (&APB_DOMAIN.USART4)
#define STM_USART5        (&APB_DOMAIN.USART5)
#define STM_I2C1          (&APB_DOMAIN.I2C1)
#define STM_I2C2          (&APB_DOMAIN.I2C2)
#define STM_USB           (&APB_DOMAIN.USB)
#define STM_CAN_USB_SRAM  (APB_DOMAIN.CAN_USB_SRAM)
#define STM_CAN           (&APB_DOMAIN.CAN)
#define STM_CRS           (&APB_DOMAIN.CRS)
#define STM_PWR           (&APB_DOMAIN.PWR)
#define STM_DAC           (&APB_DOMAIN.DAC)
#define STM_CEC           (&APB_DOMAIN.CEC)
#define STM_SYSCFG        (&APB_DOMAIN.SYSCFG)
#define STM_COMP          (&APB_DOMAIN.SYSCFG.COMP)
#define STM_EXTI          (&APB_DOMAIN.EXTI)
#define STM_USART6        (&APB_DOMAIN.USART6)
#define STM_USART7        (&APB_DOMAIN.USART7)
#define STM_USART8        (&APB_DOMAIN.USART8)
#define STM_ADC           (&APB_DOMAIN.ADC)
#define STM_TIM1          (&APB_DOMAIN.TIM1)
#define STM_SPI1          (&APB_DOMAIN.SPI1)
#define STM_USART1        (&APB_DOMAIN.USART1)
#define STM_TIM15         (&APB_DOMAIN.TIM15)
#define STM_TIM16         (&APB_DOMAIN.TIM16)
#define STM_TIM17         (&APB_DOMAIN.TIM17)
#define STM_DBGMCU        (&APB_DOMAIN.DBGMCU)

#define STM_DMA1          (&AHB1_DOMAIN.DMA1)
#define STM_DMA2          (&AHB1_DOMAIN.DMA2)
#define STM_RCC           (&AHB1_DOMAIN.RCC)
#define STM_FLASH         (&AHB1_DOMAIN.FLASH)
#define STM_CRC           (&AHB1_DOMAIN.CRC)
#define STM_TSC           (&AHB1_DOMAIN.TSC)

#define STM_GPIOA         (&AHB2_DOMAIN.GPIO[0])
#define STM_GPIOB         (&AHB2_DOMAIN.GPIO[1])
#define STM_GPIOC         (&AHB2_DOMAIN.GPIO[2])
#define STM_GPIOD         (&AHB2_DOMAIN.GPIO[3])
#define STM_GPIOE         (&AHB2_DOMAIN.GPIO[4])
#define STM_GPIOF         (&AHB2_DOMAIN.GPIO[5])
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_STM32F0XX_PLATFORM_DEFS_H_ */
