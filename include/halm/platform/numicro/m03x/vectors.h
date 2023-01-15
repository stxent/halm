/*
 * halm/platform/numicro/m03x/vectors.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_NUMICRO_M03X_VECTORS_H_
#define HALM_PLATFORM_NUMICRO_M03X_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  BOD_IRQ           = 0,
  WDT_IRQ           = 1,
  EINT024_IRQ       = 2,
  EINT135_IRQ       = 3,
  GPIO_PAPBPGPH_IRQ = 4,
  GPIO_PCPDPEPF_IRQ = 5,
  PWM0_IRQ          = 6,
  PWM1_IRQ          = 7,
  TMR0_IRQ          = 8,
  TMR1_IRQ          = 9,
  TMR2_IRQ          = 10,
  TMR3_IRQ          = 11,
  UART02_IRQ        = 12,
  UART13_IRQ        = 13,
  SPI0_IRQ          = 14,
  QSPI0_IRQ         = 15,
  UART57_IRQ        = 17,
  I2C0_IRQ          = 18,
  I2C1_IRQ          = 19,
  BPWM0_IRQ         = 20,
  BPWM1_IRQ         = 21,
  USCI01_IRQ        = 22,
  USBD_IRQ          = 23,
  ACMP01_IRQ        = 25,
  PDMA_IRQ          = 26,
  UART46_IRQ        = 27,
  PWRWU_IRQ         = 28,
  ADC_IRQ           = 29,
  CKFAIL_IRQ        = 30,
  RTC_IRQ           = 31
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_M03X_VECTORS_H_ */
