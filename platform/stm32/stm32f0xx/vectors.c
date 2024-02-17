/*
 * vectors.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
void defaultHandler(void) __attribute__((weak));
static void emptyHandler(void);
/*----------------------------------------------------------------------------*/
/* Core Cortex-M0 IRQ handlers */
void RESET_ISR(void) __attribute__((weak, alias("defaultHandler")));
void NMI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void HARDFAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SVCALL_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PENDSV_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SYSTICK_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
/* Chip-specific IRQ handlers */
void WWDG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PVD_VDDIO2_ISR(void);
void RTC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLASH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RCC_CRS_ISR(void);
void EXTI0_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EXTI2_3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EXTI4_15_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TSC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA1_CHANNEL1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void ADC1_COMP_ISR(void);
void TIM1_BRK_UP_TRG_COM_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void TIM1_CC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM6_DAC_ISR(void);
void TIM7_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM14_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM15_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM16_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIM17_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART3_8_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CEC_CAN_ISR(void);
void USB_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
/* Virtual IRQ handlers */
void PVD_ISR(void) __attribute__((weak, alias("emptyHandler")));
void VDDIO2_ISR(void) __attribute__((weak, alias("emptyHandler")));
void RCC_ISR(void) __attribute__((weak, alias("emptyHandler")));
void CRS_ISR(void) __attribute__((weak, alias("emptyHandler")));
void ADC1_ISR(void) __attribute__((weak, alias("emptyHandler")));
void COMP_ISR(void) __attribute__((weak, alias("emptyHandler")));
void TIM6_ISR(void) __attribute__((weak, alias("emptyHandler")));
void DAC_ISR(void) __attribute__((weak, alias("emptyHandler")));
void CEC_ISR(void) __attribute__((weak, alias("emptyHandler")));
void CAN_ISR(void) __attribute__((weak, alias("emptyHandler")));
/*----------------------------------------------------------------------------*/
extern unsigned long _stack; /* Initial stack pointer */
/*----------------------------------------------------------------------------*/
__attribute__((section(".vectors"))) void (* const vectorTable[])(void) = {
    /* The top of the stack */
    (void (*)(void))(unsigned long)&_stack,
    /* Core interrupts */
    RESET_ISR,
    NMI_ISR,
    HARDFAULT_ISR,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SVCALL_ISR,
    NULL,
    NULL,
    PENDSV_ISR,
    SYSTICK_ISR,

    /* Chip-specific interrupts */
    WWDG_ISR,
    PVD_VDDIO2_ISR,
    RTC_ISR,
    FLASH_ISR,
    RCC_CRS_ISR,
    EXTI0_1_ISR,
    EXTI2_3_ISR,
    EXTI4_15_ISR,
    TSC_ISR,
    DMA1_CHANNEL1_ISR,
    DMA1_CHANNEL2_3_DMA2_CHANNEL1_2_ISR,
    DMA1_CHANNEL4_7_DMA2_CHANNEL3_5_ISR,
    ADC1_COMP_ISR,
    TIM1_BRK_UP_TRG_COM_ISR,
    TIM1_CC_ISR,
    TIM2_ISR,
    TIM3_ISR,
    TIM6_DAC_ISR,
    TIM7_ISR,
    TIM14_ISR,
    TIM15_ISR,
    TIM16_ISR,
    TIM17_ISR,
    I2C1_ISR,
    I2C2_ISR,
    SPI1_ISR,
    SPI2_ISR,
    USART1_ISR,
    USART2_ISR,
    USART3_8_ISR,
    CEC_CAN_ISR,
    USB_ISR
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
void PVD_VDDIO2_ISR(void)
{
  PVD_ISR();
  VDDIO2_ISR();
}
/*----------------------------------------------------------------------------*/
void RCC_CRS_ISR(void)
{
  RCC_ISR();
  CRS_ISR();
}
/*----------------------------------------------------------------------------*/
void ADC1_COMP_ISR(void)
{
  ADC1_ISR();
  COMP_ISR();
}
/*----------------------------------------------------------------------------*/
void TIM6_DAC_ISR(void)
{
  TIM6_ISR();
  DAC_ISR();
}
/*----------------------------------------------------------------------------*/
void CEC_CAN_ISR(void)
{
  CEC_ISR();
  CAN_ISR();
}
