/*
 * vectors.c
 * Copyright (C) 2020 xent
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
void RIT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT16B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT16B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT32B0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void CT32B1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_FIQ_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void WWDT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void BOD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void FLASH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB_WAKEUP_ISR(void) __attribute__((weak, alias("defaultHandler")));
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
    RIT_ISR,
    0,
    SSP1_ISR,
    I2C_ISR,
    CT16B0_ISR,
    CT16B1_ISR,
    CT32B0_ISR,
    CT32B1_ISR,
    SSP0_ISR,
    USART_ISR,
    USB_ISR,
    USB_FIQ_ISR,
    ADC_ISR,
    WWDT_ISR,
    BOD_ISR,
    FLASH_ISR,
    0,
    0,
    USB_WAKEUP_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
