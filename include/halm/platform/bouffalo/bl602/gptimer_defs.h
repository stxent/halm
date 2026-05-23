/*
 * halm/platform/bouffalo/bl602/gptimer_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_GPTIMER_DEFS_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_BOUFFALO_BL602_GPTIMER_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_GPTIMER_DEFS_H_
/*------------------Timer Clock Source Configuration registers----------------*/
enum
{
  CS_FCLK       = 0,
  CS_F32K_CLK   = 1,
  CS_1KHZ_CLK   = 2,
  CS_32MHZ_CLK  = 3
};

#define TCCR_CS1(value)                 BIT_FIELD((value), 2)
#define TCCR_CS1_MASK                   BIT_FIELD(MASK(2), 2)
#define TCCR_CS1_VALUE(reg)             FIELD_VALUE((reg), TCCR_CS1_MASK, 2)

#define TCCR_CS2(value)                 BIT_FIELD((value), 5)
#define TCCR_CS2_MASK                   BIT_FIELD(MASK(2), 5)
#define TCCR_CS2_VALUE(reg)             FIELD_VALUE((reg), TCCR_CS2_MASK, 5)

#define TCCR_CSWDT(value)               BIT_FIELD((value), 8)
#define TCCR_CSWDT_MASK                 BIT_FIELD(MASK(2), 8)
#define TCCR_CSWDT_VALUE(reg)           FIELD_VALUE((reg), TCCR_CSWDT_MASK, 8)

#define TCCR_CS(channel, value)         BIT_FIELD((value), (channel * 3) + 2)
#define TCCR_CS_MASK(channel)           BIT_FIELD(MASK(2), (channel * 3) + 2)
#define TCCR_CS_VALUE(channel, reg) \
    FIELD_VALUE((reg), TCCR_CS_MASK(channel), (channel * 3) + 2)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_GPTIMER_DEFS_H_ */
