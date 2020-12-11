/*
 * halm/platform/lpc/lpc13xx/vectors.h
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13XX_VECTORS_H_
#define HALM_PLATFORM_LPC_LPC13XX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  WAKEUP_IRQ  = 0, /* 40 start logic wake-up interrupts */
  I2C_IRQ     = 40,
  CT16B0_IRQ  = 41,
  CT16B1_IRQ  = 42,
  CT32B0_IRQ  = 43,
  CT32B1_IRQ  = 44,
  SSP0_IRQ    = 45,
  UART_IRQ    = 46,
  USB_IRQ     = 47,
  USB_FIQ_IRQ = 48,
  ADC_IRQ     = 49,
  WDT_IRQ     = 50,
  BOD_IRQ     = 51,
  FMC_IRQ     = 52,
  PIOINT3_IRQ = 53,
  PIOINT2_IRQ = 54,
  PIOINT1_IRQ = 55,
  PIOINT0_IRQ = 56,
  SSP1_IRQ    = 57
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13XX_VECTORS_H_ */
