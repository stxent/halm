/*
 * halm/platform/lpc/lpc13uxx/vectors.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_VECTORS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC13UXX_VECTORS_H_
#define HALM_PLATFORM_LPC_LPC13UXX_VECTORS_H_
/*----------------------------------------------------------------------------*/
enum
{
  /* Chip-specific interrupt sources */
  PIN_INT0_IRQ    = 0,
  PIN_INT1_IRQ    = 1,
  PIN_INT2_IRQ    = 2,
  PIN_INT3_IRQ    = 3,
  PIN_INT4_IRQ    = 4,
  PIN_INT5_IRQ    = 5,
  PIN_INT6_IRQ    = 6,
  PIN_INT7_IRQ    = 7,
  PIN_GINT0_IRQ   = 8,
  PIN_GINT1_IRQ   = 9,
  RIT_IRQ         = 12,
  SSP1_IRQ        = 14,
  I2C_IRQ         = 15,
  CT16B0_IRQ      = 16,
  CT16B1_IRQ      = 17,
  CT32B0_IRQ      = 18,
  CT32B1_IRQ      = 19,
  SSP0_IRQ        = 20,
  USART_IRQ       = 21,
  USB_IRQ         = 22,
  USB_FIQ_IRQ     = 23,
  ADC_IRQ         = 24,
  WWDT_IRQ        = 25,
  BOD_IRQ         = 26,
  FLASH_IRQ       = 27,
  USB_WAKEUP_IRQ  = 30
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC13UXX_VECTORS_H_ */
