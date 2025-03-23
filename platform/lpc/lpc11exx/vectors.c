/*
 * vectors.c
 * Copyright (C) 2013 xent
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
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT5_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT6_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT7_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_GINT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_GINT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT16B0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT16B1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT32B0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CT32B1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WWDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BOD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void FLASH_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void IOH_ISR(void);
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
    NULL,
    NULL,
    NULL,
    NULL,
    SSP1_ISR,
    I2C_ISR,
    CT16B0_ISR,
    CT16B1_ISR,
    CT32B0_ISR,
    CT32B1_ISR,
    SSP0_ISR,
    USART_ISR,
    NULL,
    NULL,
    ADC_ISR,
    WWDT_ISR,
    BOD_ISR,
    FLASH_ISR,
    NULL,
    NULL,
    NULL,
    NULL,
    IOH_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
