/*
 * vectors_m0_sub.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

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
void DAC_ISR(void) __attribute__((weak, alias("defaultHandler")));
void M4CORE_ISR(void) __attribute__((weak, alias("defaultHandler")));
void GPDMA_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SGPIO_INPUT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SGPIO_MATCH_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SGPIO_SHIFT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SGPIO_POS_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void USB1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SCT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void RIT_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_GINT1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void TIMER2_ISR(void) __attribute__((weak, alias("defaultHandler")));
void PIN_INT5_ISR(void) __attribute__((weak, alias("defaultHandler")));
void MCPWM_ISR(void) __attribute__((weak, alias("defaultHandler")));
void ADC0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C0_ISR(void) __attribute__((weak, alias("defaultHandler")));
void I2C1_ISR(void) __attribute__((weak, alias("defaultHandler")));
void SPI_ISR(void) __attribute__((weak, alias("defaultHandler")));
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
void M0APP_ISR(void) __attribute__((weak, alias("defaultHandler")));
/*----------------------------------------------------------------------------*/
/* Virtual IRQ handlers */
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

    /* Chip-specific interrupts */
    DAC_ISR,
    M4CORE_ISR,
    GPDMA_ISR,
    0,
    SGPIO_INPUT_ISR,
    SGPIO_MATCH_ISR,
    SGPIO_SHIFT_ISR,
    SGPIO_POS_ISR,
    USB0_ISR,
    USB1_ISR,
    SCT_ISR,
    RIT_ISR,
    PIN_GINT1_ISR,
    TIMER1_ISR,
    TIMER2_ISR,
    PIN_INT5_ISR,
    MCPWM_ISR,
    ADC0_ISR,
    I2C0_ISR,
    I2C1_ISR,
    SPI_ISR,
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
    M0APP_ISR
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
