/*
 * vectors_m0_app.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <stddef.h>
/*----------------------------------------------------------------------------*/
void defaultHandler(void) __attribute__((weak));
static void emptyHandler(void);
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
void RTC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void M4CORE_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPDMA_ISR(void) __attribute__((weak, alias("defaultHandler")));
void EEPROM_ATIMER_ISR(void);
void ETHERNET_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SDIO_ISR(void) __attribute__((weak, alias("defaultHandler")));
void LCD_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SCT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RIT_WWDT_ISR(void);
void TIMER0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_GINT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT4_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void MCPWM_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C0_I2C1_ISR(void);
void SGPIO_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI_DAC_ISR(void);
void ADC1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SSP0_SSP1_ISR(void);
void EVENTROUTER_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void UART1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USART2_CAN1_ISR(void);
void USART3_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2S0_I2S1_QEI_ISR(void);
void CAN0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPIFI_ADCHS_ISR(void);
void M0SUB_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
/* Virtual IRQ handlers */
void EEPROM_ISR(void) __attribute__((weak, alias("emptyHandler")));
void ATIMER_ISR(void) __attribute__((weak, alias("emptyHandler")));
void RIT_ISR(void) __attribute__((weak, alias("emptyHandler")));
void WWDT_ISR(void) __attribute__((weak, alias("emptyHandler")));
void I2C0_ISR(void) __attribute__((weak, alias("emptyHandler")));
void I2C1_ISR(void) __attribute__((weak, alias("emptyHandler")));
void SPI_ISR(void) __attribute__((weak, alias("emptyHandler")));
void DAC_ISR(void) __attribute__((weak, alias("emptyHandler")));
void SSP0_ISR(void) __attribute__((weak, alias("emptyHandler")));
void SSP1_ISR(void) __attribute__((weak, alias("emptyHandler")));
void USART2_ISR(void) __attribute__((weak, alias("emptyHandler")));
void CAN1_ISR(void) __attribute__((weak, alias("emptyHandler")));
void I2S0_ISR(void) __attribute__((weak, alias("emptyHandler")));
void I2S1_ISR(void) __attribute__((weak, alias("emptyHandler")));
void QEI_ISR(void) __attribute__((weak, alias("emptyHandler")));
void SPIFI_ISR(void) __attribute__((weak, alias("emptyHandler")));
void ADCHS_ISR(void) __attribute__((weak, alias("emptyHandler")));
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
