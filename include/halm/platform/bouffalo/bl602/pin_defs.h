/*
 * halm/platform/bouffalo/bl602/pin_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_BOUFFALO_BL602_PIN_DEFS_H_
#define HALM_PLATFORM_BOUFFALO_BL602_PIN_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------GPIO Configuration Control registers----------------------*/
enum
{
  DRV_LEVEL_0,
  DRV_LEVEL_1,
  DRV_LEVEL_2,
  DRV_LEVEL_3
};

#define GPIO_CFGCTL_IE                  BIT(0) /* Input Enable */
#define GPIO_CFGCTL_SMT                 BIT(1) /* Schmitt Trigger Control */

/* Driving Control */
#define GPIO_CFGCTL_DRV(value)          BIT_FIELD((value), 2)
#define GPIO_CFGCTL_DRV_MASK            BIT_FIELD(MASK(2), 2)
#define GPIO_CFGCTL_DRV_VALUE(reg) \
    FIELD_VALUE((reg), GPIO_CFGCTL_DRV_MASK, 2)

#define GPIO_CFGCTL_PU                  BIT(4) /* Pull Up Control */
#define GPIO_CFGCTL_PD                  BIT(5) /* Pull Down Control */

/* Function Select */
#define GPIO_CFGCTL_FUNC(value)         BIT_FIELD((value), 8)
#define GPIO_CFGCTL_FUNC_MASK           BIT_FIELD(MASK(4), 8)
#define GPIO_CFGCTL_FUNC_VALUE(reg) \
    FIELD_VALUE((reg), GPIO_CFGCTL_FUNC_MASK, 8)

#define GPIO_CFGCTL_MASK                MASK(16)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_BOUFFALO_BL602_PIN_DEFS_H_ */
