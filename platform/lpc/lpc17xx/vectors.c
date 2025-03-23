/*
 * vectors.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void defaultHandler(void);
/*----------------------------------------------------------------------------*/
/* Core Cortex-M3 IRQ handlers */
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
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PWM1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PLL0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RTC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EINT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EINT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EINT2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EINT3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BOD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPDMA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2S_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ENET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RIT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MCPWM_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void QEI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PLL1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB_ACT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN_ACT_ISR(void);
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
