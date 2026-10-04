/*
 * halm/platform/bouffalo/pwm_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_PWM_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_PWM_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* PWM pin function */
#define PIN_PWM_FUNCTION                8
/*------------------Interrupt Configuration register--------------------------*/
/* PWM channel interrupt status */
#define INT_CONFIG_INTSTS_MASK          BIT_FIELD(MASK(6), 0)
#define INT_CONFIG_INTSTS(value)        BIT_FIELD(value, 0)
#define INT_CONFIG_INTSTS_VALUE(reg) \
    FIELD_VALUE(reg, INT_CONFIG_INTSTS_MASK, 0)

/* PWM channel interrupt clear */
#define INT_CONFIG_INTCLR_MASK          BIT_FIELD(MASK(6), 8)
#define INT_CONFIG_INTCLR(value)        BIT_FIELD(value, 8)
#define INT_CONFIG_INTCLR_VALUE(reg) \
    FIELD_VALUE(reg, INT_CONFIG_INTCLR_MASK, 8)
/*------------------Clock Division configuration register---------------------*/
/* PWM clock division */
#define CLKDIV_CLKDIV_MASK              BIT_FIELD(MASK(16), 0)
#define CLKDIV_CLKDIV(value)            BIT_FIELD(value, 0)
#define CLKDIV_CLKDIV_VALUE(reg)        FIELD_VALUE(reg, CLKDIV_CLKDIV_MASK, 0)
/*------------------First counter Threshold configuration register------------*/
/* PWM first counter threshold, can't be larger than pwm_thre2 */
#define THRE1_THRE1_MASK                BIT_FIELD(MASK(16), 0)
#define THRE1_THRE1(value)              BIT_FIELD(value, 0)
#define THRE1_THRE1_VALUE(reg)          FIELD_VALUE(reg, THRE1_THRE1_MASK, 0)
/*------------------Second counter Threshold configuration register-----------*/
/* PWM second counter threshold, can't be smaller than pwm_thre1 */
#define THRE2_THRE2_MASK                BIT_FIELD(MASK(16), 0)
#define THRE2_THRE2(value)              BIT_FIELD(value, 0)
#define THRE2_THRE2_VALUE(reg)          FIELD_VALUE(reg, THRE2_THRE2_MASK, 0)
/*------------------Period Settings register----------------------------------*/
/* PWM period setting */
#define PERIOD_PERIOD_MASK              BIT_FIELD(MASK(16), 0)
#define PERIOD_PERIOD(value)            BIT_FIELD(value, 0)
#define PERIOD_PERIOD_VALUE(reg)        FIELD_VALUE(reg, PERIOD_PERIOD_MASK, 0)
/*------------------Configuration register------------------------------------*/
enum
{
  CLKSEL_XCLK = 0,
  CLKSEL_BCLK = 1,
  CLKSEL_F32K = 2
};

/* PWM clock source select, 2'b00 - xclk, 2'b01 - bclk, others - f32k_clk */
#define CONFIG_CLKSEL_MASK              BIT_FIELD(MASK(2), 0)
#define CONFIG_CLKSEL(value)            BIT_FIELD(value, 0)
#define CONFIG_CLKSEL_VALUE(reg)        FIELD_VALUE(reg, CONFIG_CLKSEL_MASK, 0)

/* PWM invert output mode */
#define CONFIG_OUTINV                   BIT(2)
/* PWM stop mode, 1'b1 - graceful, 1'b0 - abrupt */
#define CONFIG_STOPMODE                 BIT(3)
/* Software Force Value */
#define CONFIG_SWFVAL                   BIT(4)
/* Software Control Mode Enable */
#define CONFIG_SWMODE                   BIT(5)
/* PWM stop enable */
#define CONFIG_STOPEN                   BIT(6)
/* PWM stop status */
#define CONFIG_STOPSTA                  BIT(7)
/*------------------Interrupt register----------------------------------------*/
/* PWM interrupt period counter threshold */
#define INTERRUPT_INTPECN_MASK          BIT_FIELD(MASK(16), 0)
#define INTERRUPT_INTPECN(value)        BIT_FIELD(value, 0)
#define INTERRUPT_INTPECN_VALUE(reg) \
    FIELD_VALUE(reg, INTERRUPT_INTPECN_MASK, 0)

/* PWM interrupt enable */
#define INTERRUPT_INTEN                 BIT(16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_PWM_DEFS_H_ */
