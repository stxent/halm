/*
 * vectors.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void defaultHandler(void);
/*----------------------------------------------------------------------------*/
/* Core Cortex-M0+ IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RESET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void NMI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void HARDFAULT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SVCALL_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PENDSV_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SYSTICK_ISR(void);
/*----------------------------------------------------------------------------*/
/* Chip-specific IRQ handlers */
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SCT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MRT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CMP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WWDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void BOD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void FLASH_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WKT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_SEQA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_SEQB_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_THCMP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC_OVR_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DMA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT5_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT6_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT7_ISR(void);
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

    /* Platform-specific interrupts */
    SPI0_ISR,
    SPI1_ISR,
    NULL,
    UART0_ISR,
    UART1_ISR,
    UART2_ISR,
    NULL,
    I2C1_ISR,
    I2C0_ISR,
    SCT_ISR,
    MRT_ISR,
    CMP_ISR,
    WWDT_ISR,
    BOD_ISR,
    FLASH_ISR,
    WKT_ISR,
    ADC_SEQA_ISR,
    ADC_SEQB_ISR,
    ADC_THCMP_ISR,
    ADC_OVR_ISR,
    DMA_ISR,
    I2C2_ISR,
    I2C3_ISR,
    NULL,
    PIN_INT0_ISR,
    PIN_INT1_ISR,
    PIN_INT2_ISR,
    PIN_INT3_ISR,
    PIN_INT4_ISR,
    PIN_INT5_ISR,
    PIN_INT6_ISR,
    PIN_INT7_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
