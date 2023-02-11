/*
 * halm/platform/numicro/m48x/vectors.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M48X_VECTORS_H_
#define HALM_PLATFORM_NUMICRO_M48X_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  BOD_IRQ       = 0,
  IRC_IRQ       = 1,
  PWRWU_IRQ     = 2,
  SRAM_PERR_IRQ = 3,
  CLKFAIL_IRQ   = 4,
  /* Reserved */
  RTC_IRQ       = 6,
  TAMPER_IRQ    = 7,
  WDT_IRQ       = 8,
  WWDT_IRQ      = 9,
  EINT0_IRQ     = 10,
  EINT1_IRQ     = 11,
  EINT2_IRQ     = 12,
  EINT3_IRQ     = 13,
  EINT4_IRQ     = 14,
  EINT5_IRQ     = 15,
  GPA_IRQ       = 16,
  GPB_IRQ       = 17,
  GPC_IRQ       = 18,
  GPD_IRQ       = 19,
  GPE_IRQ       = 20,
  GPF_IRQ       = 21,
  QSPI0_IRQ     = 22,
  SPI0_IRQ      = 23,
  BRAKE0_IRQ    = 24,
  EPWM0_P0_IRQ  = 25,
  EPWM0_P1_IRQ  = 26,
  EPWM0_P2_IRQ  = 27,
  BRAKE1_IRQ    = 28,
  EPWM1_P0_IRQ  = 29,
  EPWM1_P1_IRQ  = 30,
  EPWM1_P2_IRQ  = 31,
  TMR0_IRQ      = 32,
  TMR1_IRQ      = 33,
  TMR2_IRQ      = 34,
  TMR3_IRQ      = 35,
  UART0_IRQ     = 36,
  UART1_IRQ     = 37,
  I2C0_IRQ      = 38,
  I2C1_IRQ      = 39,
  PDMA_IRQ      = 40,
  DAC_IRQ       = 41,
  EADC0_P0_IRQ  = 42,
  EADC0_P1_IRQ  = 43,
  ACMP01_IRQ    = 44,
  /* Reserved */
  EADC0_P2_IRQ  = 46,
  EADC0_P3_IRQ  = 47,
  UART2_IRQ     = 48,
  UART3_IRQ     = 49,
  QSPI1_IRQ     = 50,
  SPI1_IRQ      = 51,
  SPI2_IRQ      = 52,
  USBD_IRQ      = 53,
  USBH_IRQ      = 54,
  USBOTG_IRQ    = 55,
  CAN0_IRQ      = 56,
  CAN1_IRQ      = 57,
  SC0_IRQ       = 58,
  SC1_IRQ       = 59,
  SC2_IRQ       = 60,
  /* Reserved */
  SPI3_IRQ      = 62,
  /* Reserved */
  SDHOST0_IRQ   = 64,
  HSUSBD_IRQ    = 65,
  EMAC_TX_IRQ   = 66,
  EMAC_RX_IRQ   = 67,
  I2S0_IRQ      = 68,
  /* Reserved */
  OPA0_IRQ      = 70,
  CRYPTO_IRQ    = 71,
  GPG_IRQ       = 72,
  EINT6_IRQ     = 73,
  UART4_IRQ     = 74,
  UART5_IRQ     = 75,
  USCI0_IRQ     = 76,
  USCI1_IRQ     = 77,
  BPWM0_IRQ     = 78,
  BPWM1_IRQ     = 79,
  SPIM_IRQ      = 80,
  CCAP_IRQ      = 81,
  I2C2_IRQ      = 82,
  /* Reserved */
  QEI0_IRQ      = 84,
  QEI1_IRQ      = 85,
  ECAP0_IRQ     = 86,
  ECAP1_IRQ     = 87,
  GPH_IRQ       = 88,
  EINT7_IRQ     = 89,
  SDHOST1_IRQ   = 90,
  /* Reserved */
  HSUSBH_IRQ    = 92,
  HSOTG_IRQ     = 93,
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  /* Reserved */
  TRNG_IRQ      = 101,
  UART6_IRQ     = 102,
  UART7_IRQ     = 103,
  EADC1_P0_IRQ  = 104,
  EADC1_P1_IRQ  = 105,
  EADC1_P2_IRQ  = 106,
  EADC1_P3_IRQ  = 107,
  CAN2_IRQ      = 108
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M48X_VECTORS_H_ */
