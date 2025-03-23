/*
 * vectors_m0_app.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void defaultHandler(void);
static void emptyHandler(void);
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
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void RTC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void M4CORE_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void GPDMA_ISR(void);
void EEPROM_ATIMER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ETHERNET_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SDIO_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void LCD_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USB1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SCT_ISR(void);
void RIT_WWDT_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_GINT1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void PIN_INT4_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void TIMER3_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void MCPWM_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC0_ISR(void);
void I2C0_I2C1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void SGPIO_ISR(void);
void SPI_DAC_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void ADC1_ISR(void);
void SSP0_SSP1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void EVENTROUTER_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART0_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void UART1_ISR(void);
void USART2_CAN1_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void USART3_ISR(void);
void I2S0_I2S1_QEI_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void CAN0_ISR(void);
void SPIFI_ADCHS_ISR(void);
[[gnu::weak]] [[gnu::alias("defaultHandler")]] void M0SUB_ISR(void);
/*----------------------------------------------------------------------------*/
/* Virtual IRQ handlers */
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void EEPROM_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void ATIMER_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void RIT_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void WWDT_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void I2C0_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void I2C1_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void SPI_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void DAC_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void SSP0_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void SSP1_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void USART2_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void CAN1_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void I2S0_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void I2S1_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void QEI_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void SPIFI_ISR(void);
[[gnu::weak]] [[gnu::alias("emptyHandler")]] void ADCHS_ISR(void);
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
    RTC_ISR,
    M4CORE_ISR,
    GPDMA_ISR,
    NULL,
    EEPROM_ATIMER_ISR,
    ETHERNET_ISR,
    SDIO_ISR,
    LCD_ISR,
    USB0_ISR,
    USB1_ISR,
    SCT_ISR,
    RIT_WWDT_ISR,
    TIMER0_ISR,
    PIN_GINT1_ISR,
    PIN_INT4_ISR,
    TIMER3_ISR,
    MCPWM_ISR,
    ADC0_ISR,
    I2C0_I2C1_ISR,
    SGPIO_ISR,
    SPI_DAC_ISR,
    ADC1_ISR,
    SSP0_SSP1_ISR,
    EVENTROUTER_ISR,
    USART0_ISR,
    UART1_ISR,
    USART2_CAN1_ISR,
    USART3_ISR,
    I2S0_I2S1_QEI_ISR,
    CAN0_ISR,
    SPIFI_ADCHS_ISR,
    M0SUB_ISR
};
/*----------------------------------------------------------------------------*/
void defaultHandler(void)
{
  while (1);
}
/*----------------------------------------------------------------------------*/
static void emptyHandler(void)
{
}
/*----------------------------------------------------------------------------*/
void EEPROM_ATIMER_ISR(void)
{
  EEPROM_ISR();
  ATIMER_ISR();
}
/*----------------------------------------------------------------------------*/
void RIT_WWDT_ISR(void)
{
  RIT_ISR();
  WWDT_ISR();
}
/*----------------------------------------------------------------------------*/
void I2C0_I2C1_ISR(void)
{
  I2C0_ISR();
  I2C1_ISR();
}
/*----------------------------------------------------------------------------*/
void SPI_DAC_ISR(void)
{
  SPI_ISR();
  DAC_ISR();
}
/*----------------------------------------------------------------------------*/
void SSP0_SSP1_ISR(void)
{
  SSP0_ISR();
  SSP1_ISR();
}
/*----------------------------------------------------------------------------*/
void USART2_CAN1_ISR(void)
{
  USART2_ISR();
  CAN1_ISR();
}
/*----------------------------------------------------------------------------*/
void I2S0_I2S1_QEI_ISR(void)
{
  I2S0_ISR();
  I2S1_ISR();
  QEI_ISR();
}
/*----------------------------------------------------------------------------*/
void SPIFI_ADCHS_ISR(void)
{
  SPIFI_ISR();
  ADCHS_ISR();
}
