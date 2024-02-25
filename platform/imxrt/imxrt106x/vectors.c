/*
 * vectors.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
void defaultHandler(void) __attribute__((weak));
/*----------------------------------------------------------------------------*/
/* Core Cortex-M4 IRQ handlers */
void RESET_ISR(void) __attribute__((weak, alias("defaultHandler")));
void NMI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void HARDFAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void MEMMANAGE_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BUSFAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USAGEFAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SVCALL_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DEBUGMON_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PENDSV_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SYSTICK_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
/* Chip-specific IRQ handlers */
void DMA0_DMA16_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA1_DMA17_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA2_DMA18_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA3_DMA19_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA4_DMA20_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA5_DMA21_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA6_DMA22_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA7_DMA23_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA8_DMA24_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA9_DMA25_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA10_DMA26_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA11_DMA27_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA12_DMA28_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA13_DMA29_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA14_DMA30_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA15_DMA31_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DMA_ERROR_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CTI0_ERROR_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CTI1_ERROR_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CORE_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART5_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART6_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART7_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPUART8_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPI2C1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPI2C2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPI2C3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPI2C4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPSPI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPSPI2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPSPI3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LPSPI4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLEXRAM_ISR(void) __attribute__((weak, alias("defaultHandler")));
void KPP_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TSC_DIG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPR_ISR_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LCDIF_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CSI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PXP_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDOG2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SNVS_HP_WRAPPER_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SNVS_HP_WRAPPER_TZ_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void SNVS_LP_WRAPPER_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CSU_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DCP_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DCP_VMI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TRNG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SJC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BEE_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SAI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SAI2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SAI3_RX_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SAI3_TX_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPDIF_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PMU_EVENT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TEMP_LOW_HIGH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TEMP_PANIC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_PHY1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_PHY2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DCDC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO10_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT5_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT6_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_INT7_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO1_COMBINED_0_15_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO1_COMBINED_16_31_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO2_COMBINED_0_15_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO2_COMBINED_16_31_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO3_COMBINED_0_15_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO3_COMBINED_16_31_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO4_COMBINED_0_15_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO4_COMBINED_16_31_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO5_COMBINED_0_15_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void GPIO5_COMBINED_16_31_ISR(void)
    __attribute__((weak, alias("defaultHandler")));
void FLEXIO1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLEXIO2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDOG1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RTWDOG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EWM_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CCM_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CCM_2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SRC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPT2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_FAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLEXSPI2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLEXSPI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SEMC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USDHC1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USDHC2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_OTG2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_OTG1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENET_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENET_1588_TIMER_ISR(void) __attribute__((weak, alias("defaultHandler")));
void XBAR1_ISR_0_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void XBAR1_ISR_2_3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ETC_ISR0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ETC_ISR1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ETC_ISR2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ETC_ERROR_ISR_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ACMP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ACMP2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ACMP3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ACMP4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENC1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENC2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENC3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENC4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM2_0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM2_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM2_2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM2_3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM2_FAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM3_0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM3_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM3_2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM3_3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM3_FAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM4_0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM4_1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM4_2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM4_3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM4_FAULT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENET2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENET2_1588_TIMER_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLEXIO3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO6_7_8_9_ISR(void) __attribute__((weak, alias("defaultHandler")));
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
    DMA0_DMA16_ISR,
    DMA1_DMA17_ISR,
    DMA2_DMA18_ISR,
    DMA3_DMA19_ISR,
    DMA4_DMA20_ISR,
    DMA5_DMA21_ISR,
    DMA6_DMA22_ISR,
    DMA7_DMA23_ISR,
    DMA8_DMA24_ISR,
    DMA9_DMA25_ISR,
    DMA10_DMA26_ISR,
    DMA11_DMA27_ISR,
    DMA12_DMA28_ISR,
    DMA13_DMA29_ISR,
    DMA14_DMA30_ISR,
    DMA15_DMA31_ISR,
    DMA_ERROR_ISR,
    CTI0_ERROR_ISR,
    CTI1_ERROR_ISR,
    CORE_ISR,
    LPUART1_ISR,
    LPUART2_ISR,
    LPUART3_ISR,
    LPUART4_ISR,
    LPUART5_ISR,
    LPUART6_ISR,
    LPUART7_ISR,
    LPUART8_ISR,
    LPI2C1_ISR,
    LPI2C2_ISR,
    LPI2C3_ISR,
    LPI2C4_ISR,
    LPSPI1_ISR,
    LPSPI2_ISR,
    LPSPI3_ISR,
    LPSPI4_ISR,
    CAN1_ISR,
    CAN2_ISR,
    FLEXRAM_ISR,
    KPP_ISR,
    TSC_DIG_ISR,
    GPR_ISR_ISR,
    LCDIF_ISR,
    CSI_ISR,
    PXP_ISR,
    WDOG2_ISR,
    SNVS_HP_WRAPPER_ISR,
    SNVS_HP_WRAPPER_TZ_ISR,
    SNVS_LP_WRAPPER_ISR,
    CSU_ISR,
    DCP_ISR,
    DCP_VMI_ISR,
    NULL,
    TRNG_ISR,
    SJC_ISR,
    BEE_ISR,
    SAI1_ISR,
    SAI2_ISR,
    SAI3_RX_ISR,
    SAI3_TX_ISR,
    SPDIF_ISR,
    PMU_EVENT_ISR,
    NULL,
    TEMP_LOW_HIGH_ISR,
    TEMP_PANIC_ISR,
    USB_PHY1_ISR,
    USB_PHY2_ISR,
    ADC1_ISR,
    ADC2_ISR,
    DCDC_ISR,
    NULL,
    GPIO10_ISR,
    GPIO1_INT0_ISR,
    GPIO1_INT1_ISR,
    GPIO1_INT2_ISR,
    GPIO1_INT3_ISR,
    GPIO1_INT4_ISR,
    GPIO1_INT5_ISR,
    GPIO1_INT6_ISR,
    GPIO1_INT7_ISR,
    GPIO1_COMBINED_0_15_ISR,
    GPIO1_COMBINED_16_31_ISR,
    GPIO2_COMBINED_0_15_ISR,
    GPIO2_COMBINED_16_31_ISR,
    GPIO3_COMBINED_0_15_ISR,
    GPIO3_COMBINED_16_31_ISR,
    GPIO4_COMBINED_0_15_ISR,
    GPIO4_COMBINED_16_31_ISR,
    GPIO5_COMBINED_0_15_ISR,
    GPIO5_COMBINED_16_31_ISR,
    FLEXIO1_ISR,
    FLEXIO2_ISR,
    WDOG1_ISR,
    RTWDOG_ISR,
    EWM_ISR,
    CCM_1_ISR,
    CCM_2_ISR,
    GPC_ISR,
    SRC_ISR,
    NULL,
    GPT1_ISR,
    GPT2_ISR,
    PWM1_0_ISR,
    PWM1_1_ISR,
    PWM1_2_ISR,
    PWM1_3_ISR,
    PWM1_FAULT_ISR,
    FLEXSPI2_ISR,
    FLEXSPI_ISR,
    SEMC_ISR,
    USDHC1_ISR,
    USDHC2_ISR,
    USB_OTG2_ISR,
    USB_OTG1_ISR,
    ENET_ISR,
    ENET_1588_TIMER_ISR,
    XBAR1_ISR_0_1_ISR,
    XBAR1_ISR_2_3_ISR,
    ADC_ETC_ISR0_ISR,
    ADC_ETC_ISR1_ISR,
    ADC_ETC_ISR2_ISR,
    ADC_ETC_ERROR_ISR_ISR,
    PIT_ISR,
    ACMP1_ISR,
    ACMP2_ISR,
    ACMP3_ISR,
    ACMP4_ISR,
    NULL,
    NULL,
    ENC1_ISR,
    ENC2_ISR,
    ENC3_ISR,
    ENC4_ISR,
    TMR1_ISR,
    TMR2_ISR,
    TMR3_ISR,
    TMR4_ISR,
    PWM2_0_ISR,
    PWM2_1_ISR,
    PWM2_2_ISR,
    PWM2_3_ISR,
    PWM2_FAULT_ISR,
    PWM3_0_ISR,
    PWM3_1_ISR,
    PWM3_2_ISR,
    PWM3_3_ISR,
    PWM3_FAULT_ISR,
    PWM4_0_ISR,
    PWM4_1_ISR,
    PWM4_2_ISR,
    PWM4_3_ISR,
    PWM4_FAULT_ISR,
    ENET2_ISR,
    ENET2_1588_TIMER_ISR,
    CAN3_ISR,
    NULL,
    FLEXIO3_ISR,
    GPIO6_7_8_9_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
