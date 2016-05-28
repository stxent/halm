/*
 * vectors.c
 * Copyright (C) 2016 xent
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
// TODO
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

    // TODO
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
