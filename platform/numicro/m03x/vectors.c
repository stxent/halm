/*
 * vectors.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void defaultHandler(void);
/*----------------------------------------------------------------------------*/
/* Core Cortex-M0 IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RESET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void NMI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void HARDFAULT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SVCALL_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PENDSV_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SYSTICK_ISR(void);
/*----------------------------------------------------------------------------*/
/* Chip-specific IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BOD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EINT024_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EINT135_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPIO_PAPBPGPH_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPIO_PCPDPEPF_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PWM0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PWM1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TMR0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TMR1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TMR2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TMR3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART02_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART13_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void QSPI0_ISR (void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART57_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BPWM0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BPWM1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USCI01_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USBD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ACMP01_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PDMA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART46_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PWRWU_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CKFAIL_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RTC_ISR(void);
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
    BOD_ISR,
    WDT_ISR,
    EINT024_ISR,
    EINT135_ISR,
    GPIO_PAPBPGPH_ISR,
    GPIO_PCPDPEPF_ISR,
    PWM0_ISR,
    PWM1_ISR,
    TMR0_ISR,
    TMR1_ISR,
    TMR2_ISR,
    TMR3_ISR,
    UART02_ISR,
    UART13_ISR,
    SPI0_ISR,
    QSPI0_ISR,
    NULL,
    UART57_ISR,
    I2C0_ISR,
    I2C1_ISR,
    BPWM0_ISR,
    BPWM1_ISR,
    USCI01_ISR,
    USBD_ISR,
    NULL,
    ACMP01_ISR,
    PDMA_ISR,
    UART46_ISR,
    PWRWU_ISR,
    ADC_ISR,
    CKFAIL_ISR,
    RTC_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
