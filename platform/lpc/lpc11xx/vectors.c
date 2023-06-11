/*
 * vectors.c
 * Copyright (C) 2013 xent
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
void WAKEUP_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CAN_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT16B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT16B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT32B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT32B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_FIQ_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIO0_ISR(void) __attribute__((weak, alias("defaultHandler")));
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

    /* Other LPC11xx interrupts */
    CAN_ISR,
    SSP1_ISR,
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
    0,
    PIO3_ISR,
    PIO2_ISR,
    PIO1_ISR,
    PIO0_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
