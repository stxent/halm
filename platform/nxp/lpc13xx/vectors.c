/*
 * vectors.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
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
void I2C_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER16B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER16B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER32B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER32B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_FIQ_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FMC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WAKEUP_ISR(void) __attribute__((weak, alias("defaultHandler")));
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

    /* PIO0 wake-up interrupts */
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,

    /* PIO1 wake-up interrupts */
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,

    /* PIO2 wake-up interrupts */
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,

    /* PIO3 wake-up interrupts */
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,
    WAKEUP_ISR,

    /* Other LPC13xx interrupts */
    I2C_ISR,
    TIMER16B0_ISR,
    TIMER16B1_ISR,
    TIMER32B0_ISR,
    TIMER32B1_ISR,
    SSP0_ISR,
    UART_ISR,
    USB_ISR,
    USB_FIQ_ISR,
    ADC_ISR,
    WDT_ISR,
    BOD_ISR,
    FMC_ISR,
    PIO3_ISR,
    PIO2_ISR,
    PIO1_ISR,
    PIO0_ISR,
    SSP1_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
