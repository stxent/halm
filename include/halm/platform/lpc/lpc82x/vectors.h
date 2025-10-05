/*
 * halm/platform/lpc/lpc82x/vectors.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_VECTORS_H_
#define HALM_PLATFORM_LPC_LPC82X_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  SPI0_IRQ      = 0,
  SPI1_IRQ      = 1,
  UART0_IRQ     = 3,
  UART1_IRQ     = 4,
  UART2_IRQ     = 5,
  I2C1_IRQ      = 7,
  I2C0_IRQ      = 8,
  SCT_IRQ       = 9,
  MRT_IRQ       = 10,
  CMP_IRQ       = 11,
  WWDT_IRQ      = 12,
  BOD_IRQ       = 13,
  FLASH_IRQ     = 14,
  WKT_IRQ       = 15,
  ADC_SEQA_IRQ  = 16,
  ADC_SEQB_IRQ  = 17,
  ADC_THCMP_IRQ = 18,
  ADC_OVR_IRQ   = 19,
  DMA_IRQ       = 20,
  I2C2_IRQ      = 21,
  I2C3_IRQ      = 22,
  PIN_INT0_IRQ  = 24,
  PIN_INT1_IRQ  = 25,
  PIN_INT2_IRQ  = 26,
  PIN_INT3_IRQ  = 27,
  PIN_INT4_IRQ  = 28,
  PIN_INT5_IRQ  = 29,
  PIN_INT6_IRQ  = 30,
  PIN_INT7_IRQ  = 31
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_VECTORS_H_ */
