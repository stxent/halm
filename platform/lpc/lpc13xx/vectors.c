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
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT16B0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT16B1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT32B0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT32B1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB_FIQ_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BOD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void FMC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIO3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIO2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIO1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIO0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WAKEUP_ISR(void);
/*----------------------------------------------------------------------------*/
extern unsigned long _stack; /* Initial stack pointer */
/*----------------------------------------------------------------------------*/
[[gnu::section(".vectors")]] void (* const vectorTable[])(void) = {
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
    CT16B0_ISR,
    CT16B1_ISR,
    CT32B0_ISR,
    CT32B1_ISR,
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
