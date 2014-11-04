/*
 * interrupts.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

__attribute__((weak)) void defaultHandler(void);
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
void PIN_INT0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT5_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT6_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT7_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_GINT0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_GINT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER16B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER16B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER32B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER32B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLASH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void IOH_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
extern void _stack(void); /* Initial stack pointer */
/*----------------------------------------------------------------------------*/
__attribute__((section(".vectors"))) void (* const vectorTable[])(void) = {
    /* Core interrupt sources */
    &_stack,
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

    PIN_INT0_ISR,
    PIN_INT1_ISR,
    PIN_INT2_ISR,
    PIN_INT3_ISR,
    PIN_INT4_ISR,
    PIN_INT5_ISR,
    PIN_INT6_ISR,
    PIN_INT7_ISR,
    PIN_GINT0_ISR,
    PIN_GINT1_ISR,
    0,
    0,
    0,
    0,
    SSP1_ISR,
    I2C_ISR,
    TIMER16B0_ISR,
    TIMER16B1_ISR,
    TIMER32B0_ISR,
    TIMER32B1_ISR,
    SSP0_ISR,
    USART_ISR,
    0,
    0,
    ADC_ISR,
    WDT_ISR,
    BOD_ISR,
    FLASH_ISR,
    0,
    0,
    0,
    0,
    IOH_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
