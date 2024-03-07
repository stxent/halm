/*
 * vectors_m4.c
 * Copyright (C) 2014, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void defaultHandler(void);
/*----------------------------------------------------------------------------*/
/* Core Cortex-M4 IRQ handlers */
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
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void DAC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void M0APP_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPDMA_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EEPROM_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ETHERNET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SDIO_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void LCD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SCT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RIT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MCPWM_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2C1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SSP1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART2_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2S0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void I2S1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SPIFI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SGPIO_ISR(void);
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
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EVENTROUTER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADCHS_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ATIMER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RTC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void WWDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void M0SUB_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void QEI_ISR(void);
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

    /* Chip-specific interrupts */
    DAC_ISR,
    M0APP_ISR,
    GPDMA_ISR,
    NULL,
    EEPROM_ISR,
    ETHERNET_ISR,
    SDIO_ISR,
    LCD_ISR,
    USB0_ISR,
    USB1_ISR,
    SCT_ISR,
    RIT_ISR,
    TIMER0_ISR,
    TIMER1_ISR,
    TIMER2_ISR,
    TIMER3_ISR,
    MCPWM_ISR,
    ADC0_ISR,
    I2C0_ISR,
    I2C1_ISR,
    SPI_ISR,
    ADC1_ISR,
    SSP0_ISR,
    SSP1_ISR,
    USART0_ISR,
    UART1_ISR,
    USART2_ISR,
    USART3_ISR,
    I2S0_ISR,
    I2S1_ISR,
    SPIFI_ISR,
    SGPIO_ISR,
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
    EVENTROUTER_ISR,
    CAN1_ISR,
    NULL,
    ADCHS_ISR,
    ATIMER_ISR,
    RTC_ISR,
    NULL,
    WWDT_ISR,
    M0SUB_ISR,
    CAN0_ISR,
    QEI_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
