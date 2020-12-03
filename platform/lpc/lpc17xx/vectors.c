/*
 * vectors.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

void defaultHandler(void) __attribute__((weak));
/*----------------------------------------------------------------------------*/
/* Core Cortex-M3 IRQ handlers */
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
void WDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PWM1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PLL0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RTC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EINT3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPDMA_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2S_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ENET_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RIT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void MCPWM_ISR(void) __attribute__((weak, alias("defaultHandler")));
void QEI_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PLL1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_ACT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN_ACT_ISR(void) __attribute__((weak, alias("defaultHandler")));
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
    MEMMANAGE_ISR,
    BUSFAULT_ISR,
    USAGEFAULT_ISR,
    0,
    0,
    0,
    0,
    SVCALL_ISR,
    DEBUGMON_ISR,
    0,
    PENDSV_ISR,
    SYSTICK_ISR,

    /* Chip-specific interrupts */
    WDT_ISR,
    TIMER0_ISR,
    TIMER1_ISR,
    TIMER2_ISR,
    TIMER3_ISR,
    UART0_ISR,
    UART1_ISR,
    UART2_ISR,
    UART3_ISR,
    PWM1_ISR,
    I2C0_ISR,
    I2C1_ISR,
    I2C2_ISR,
    SPI_ISR,
    SSP0_ISR,
    SSP1_ISR,
    PLL0_ISR,
    RTC_ISR,
    EINT0_ISR,
    EINT1_ISR,
    EINT2_ISR,
    EINT3_ISR,
    ADC_ISR,
    BOD_ISR,
    USB_ISR,
    CAN_ISR,
    GPDMA_ISR,
    I2S_ISR,
    ENET_ISR,
    RIT_ISR,
    MCPWM_ISR,
    QEI_ISR,
    PLL1_ISR,
    USB_ACT_ISR,
    CAN_ACT_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
