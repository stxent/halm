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
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void IRC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWRWU_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SRAM_PERR_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CLKFAIL_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RTC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TAMPER_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WWDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT5_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPA_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPB_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPE_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPF_ISR(void) __attribute__((weak, alias("defaultHandler")));
void QSPI0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BRAKE0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EPWM0_P0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EPWM0_P1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EPWM0_P2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BRAKE1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EPWM1_P0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EPWM1_P1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EPWM1_P2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PDMA_ISR(void) __attribute__((weak, alias("defaultHandler")));
void DAC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC0_P0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC0_P1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ACMP01_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC0_P2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC0_P3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void QSPI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USBD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USBH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USBOTG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SC0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SC1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SC2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SDHOST0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void HSUSBD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EMAC_TX_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EMAC_RX_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2S0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void OPA0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CRYPTO_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT6_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART5_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USCI0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USCI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BPWM0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BPWM1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPIM_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CCAP_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void QEI0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void QEI1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ECAP0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ECAP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT7_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SDHOST1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void HSUSBH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void HSOTG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TRNG_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART6_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART7_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC1_P0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC1_P1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC1_P2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EADC1_P3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN2_ISR(void) __attribute__((weak, alias("defaultHandler")));
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
    BOD_ISR,
    IRC_ISR,
    PWRWU_ISR,
    SRAM_PERR_ISR,
    CLKFAIL_ISR,
    NULL,
    RTC_ISR,
    TAMPER_ISR,
    WDT_ISR,
    WWDT_ISR,
    EINT0_ISR,
    EINT1_ISR,
    EINT2_ISR,
    EINT3_ISR,
    EINT4_ISR,
    EINT5_ISR,
    GPA_ISR,
    GPB_ISR,
    GPC_ISR,
    GPD_ISR,
    GPE_ISR,
    GPF_ISR,
    QSPI0_ISR,
    SPI0_ISR,
    BRAKE0_ISR,
    EPWM0_P0_ISR,
    EPWM0_P1_ISR,
    EPWM0_P2_ISR,
    BRAKE1_ISR,
    EPWM1_P0_ISR,
    EPWM1_P1_ISR,
    EPWM1_P2_ISR,
    TMR0_ISR,
    TMR1_ISR,
    TMR2_ISR,
    TMR3_ISR,
    UART0_ISR,
    UART1_ISR,
    I2C0_ISR,
    I2C1_ISR,
    PDMA_ISR,
    DAC_ISR,
    EADC0_P0_ISR,
    EADC0_P1_ISR,
    ACMP01_ISR,
    NULL,
    EADC0_P2_ISR,
    EADC0_P3_ISR,
    UART2_ISR,
    UART3_ISR,
    QSPI1_ISR,
    SPI1_ISR,
    SPI2_ISR,
    USBD_ISR,
    USBH_ISR,
    USBOTG_ISR,
    CAN0_ISR,
    CAN1_ISR,
    SC0_ISR,
    SC1_ISR,
    SC2_ISR,
    NULL,
    SPI3_ISR,
    NULL,
    SDHOST0_ISR,
    HSUSBD_ISR,
    EMAC_TX_ISR,
    EMAC_RX_ISR,
    I2S0_ISR,
    NULL,
    OPA0_ISR,
    CRYPTO_ISR,
    GPG_ISR,
    EINT6_ISR,
    UART4_ISR,
    UART5_ISR,
    USCI0_ISR,
    USCI1_ISR,
    BPWM0_ISR,
    BPWM1_ISR,
    SPIM_ISR,
    CCAP_ISR,
    I2C2_ISR,
    NULL,
    QEI0_ISR,
    QEI1_ISR,
    ECAP0_ISR,
    ECAP1_ISR,
    GPH_ISR,
    EINT7_ISR,
    SDHOST1_ISR,
    NULL,
    HSUSBH_ISR,
    HSOTG_ISR,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    TRNG_ISR,
    UART6_ISR,
    UART7_ISR,
    EADC1_P0_ISR,
    EADC1_P1_ISR,
    EADC1_P2_ISR,
    EADC1_P3_ISR,
    CAN2_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
