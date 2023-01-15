/*
 * vectors.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

void defaultHandler(void) __attribute__((weak));
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
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT024_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT135_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO_PAPBPGPH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPIO_PCPDPEPF_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TMR3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART02_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART13_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void QSPI0_ISR (void) __attribute__((weak, alias("defaultHandler")));
void UART57_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BPWM0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BPWM1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USCI01_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USBD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ACMP01_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PDMA_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART46_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWRWU_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CKFAIL_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RTC_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
extern void _stack(void); /* Initial stack pointer */
/*----------------------------------------------------------------------------*/
__attribute__((section(".vectors"))) void (* const vectorTable[])(void) = {
    /* The top of the stack */
    &_stack,
    /* Core interrupts */
    RESET_ISR,
    NMI_ISR,
    HARDFAULT_ISR,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SVCALL_ISR,
    0,
    0,
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
    0,
    UART57_ISR,
    I2C0_ISR,
    I2C1_ISR,
    BPWM0_ISR,
    BPWM1_ISR,
    USCI01_ISR,
    USBD_ISR,
    0,
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
