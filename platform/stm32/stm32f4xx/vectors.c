/*
 * vectors.c
 * Copyright (C) 2024 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void defaultHandler(void);
static void emptyHandler(void);
/*----------------------------------------------------------------------------*/
/* Core Cortex-M4 IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RESET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void NMI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void HARDFAULT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MEMMANAGE_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BUSFAULT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USAGEFAULT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SVCALL_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DEBUGMON_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PENDSV_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SYSTICK_ISR(void);
/*----------------------------------------------------------------------------*/
/* Chip-specific IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WWDG_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PVD_ISR(void);
void TAMP_STAMP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RTC_WKUP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void FLASH_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RCC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM5_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM6_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN1_TX_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN1_RX0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN1_RX1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN1_SCE_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI9_5_ISR(void);
void TIM1_BRK_TIM9_ISR(void);
void TIM1_UP_TIM10_ISR(void);
void TIM1_TRG_COM_TIM11_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM1_CC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C1_EV_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C1_ER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C2_EV_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C2_ER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EXTI15_10_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RTC_ALARM_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void OTG_FS_WKUP_ISR(void);
void TIM8_BRK_TIM12_ISR(void);
void TIM8_UP_TIM13_ISR(void);
void TIM8_TRG_COM_TIM14_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM8_CC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA1_STREAM7_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void FSMC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SDIO_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM5_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART5_ISR(void);
void TIM6_DAC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIM7_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ETH_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ETH_WKUP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN2_TX_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN2_RX0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN2_RX1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN2_SCE_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void OTG_FS_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM5_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM6_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA2_STREAM7_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART6_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C3_EV_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C3_ER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void OTG_HS_EP1_OUT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void OTG_HS_EP1_IN_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void OTG_HS_WKUP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void OTG_HS_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DCMI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CRYP_ISR(void);
void HASH_RNG_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void FPU_ISR(void);
/*----------------------------------------------------------------------------*/
/* Virtual IRQ handlers */
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TAMPER_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIMESTAMP_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM1_BRK_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM9_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM1_UP_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM10_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM1_TRG_COM_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM11_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM8_BRK_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM12_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM8_UP_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM13_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM8_TRG_COM_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM14_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void TIM6_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void DAC_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void HASH_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void RNG_ISR(void);
/*----------------------------------------------------------------------------*/
extern unsigned long _stack; /* Initial stack pointer */
/*----------------------------------------------------------------------------*/
[[gnu::section(".vectors")]] void (* const vector_table[])(void) = {
    /* The top of the stack */
    (void (*)(void))(unsigned long)&_stack,
    /* Core interrupts */
    RESET_ISR,
    NMI_ISR,
    HARDFAULT_ISR,
    MEMMANAGE_ISR,
    BUSFAULT_ISR,
    USAGEFAULT_ISR,
    NULL,
    NULL,
    NULL,
    NULL,
    SVCALL_ISR,
    DEBUGMON_ISR,
    NULL,
    PENDSV_ISR,
    SYSTICK_ISR,

    /* Chip-specific interrupts */
    WWDG_ISR,
    PVD_ISR,
    TAMP_STAMP_ISR,
    RTC_WKUP_ISR,
    FLASH_ISR,
    RCC_ISR,
    EXTI0_ISR,
    EXTI1_ISR,
    EXTI2_ISR,
    EXTI3_ISR,
    EXTI4_ISR,
    DMA1_STREAM0_ISR,
    DMA1_STREAM1_ISR,
    DMA1_STREAM2_ISR,
    DMA1_STREAM3_ISR,
    DMA1_STREAM4_ISR,
    DMA1_STREAM5_ISR,
    DMA1_STREAM6_ISR,
    ADC_ISR,
    CAN1_TX_ISR,
    CAN1_RX0_ISR,
    CAN1_RX1_ISR,
    CAN1_SCE_ISR,
    EXTI9_5_ISR,
    TIM1_BRK_TIM9_ISR,
    TIM1_UP_TIM10_ISR,
    TIM1_TRG_COM_TIM11_ISR,
    TIM1_CC_ISR,
    TIM2_ISR,
    TIM3_ISR,
    TIM4_ISR,
    I2C1_EV_ISR,
    I2C1_ER_ISR,
    I2C2_EV_ISR,
    I2C2_ER_ISR,
    SPI1_ISR,
    SPI2_ISR,
    USART1_ISR,
    USART2_ISR,
    USART3_ISR,
    EXTI15_10_ISR,
    RTC_ALARM_ISR,
    OTG_FS_WKUP_ISR,
    TIM8_BRK_TIM12_ISR,
    TIM8_UP_TIM13_ISR,
    TIM8_TRG_COM_TIM14_ISR,
    TIM8_CC_ISR,
    DMA1_STREAM7_ISR,
    FSMC_ISR,
    SDIO_ISR,
    TIM5_ISR,
    SPI3_ISR,
    UART4_ISR,
    UART5_ISR,
    TIM6_DAC_ISR,
    TIM7_ISR,
    DMA2_STREAM0_ISR,
    DMA2_STREAM1_ISR,
    DMA2_STREAM2_ISR,
    DMA2_STREAM3_ISR,
    DMA2_STREAM4_ISR,
    ETH_ISR,
    ETH_WKUP_ISR,
    CAN2_TX_ISR,
    CAN2_RX0_ISR,
    CAN2_RX1_ISR,
    CAN2_SCE_ISR,
    OTG_FS_ISR,
    DMA2_STREAM5_ISR,
    DMA2_STREAM6_ISR,
    DMA2_STREAM7_ISR,
    USART6_ISR,
    I2C3_EV_ISR,
    I2C3_ER_ISR,
    OTG_HS_EP1_OUT_ISR,
    OTG_HS_EP1_IN_ISR,
    OTG_HS_WKUP_ISR,
    OTG_HS_ISR,
    DCMI_ISR,
    CRYP_ISR,
    HASH_RNG_ISR,
    FPU_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
/*----------------------------------------------------------------------------*/
static void emptyHandler(void)
{
}
/*----------------------------------------------------------------------------*/
void TAMP_STAMP_ISR(void)
{
  TAMPER_ISR();
  TIMESTAMP_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM1_BRK_TIM9_ISR(void)
{
  TIM1_BRK_ISR();
  TIM9_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM1_UP_TIM10_ISR(void)
{
  TIM1_UP_ISR();
  TIM10_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM1_TRG_COM_TIM11_ISR(void)
{
  TIM1_TRG_COM_ISR();
  TIM11_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM8_BRK_TIM12_ISR(void)
{
  TIM8_BRK_ISR();
  TIM12_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM8_UP_TIM13_ISR(void)
{
  TIM8_UP_ISR();
  TIM13_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM8_TRG_COM_TIM14_ISR(void)
{
  TIM8_TRG_COM_ISR();
  TIM14_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM6_DAC_ISR(void)
{
  DAC_ISR();
  TIM6_ISR();
}
/*----------------------------------------------------------------------------*/
void HASH_RNG_ISR(void)
{
  HASH_ISR();
  RNG_ISR();
}
