/*
 * halm/platform/numicro/rtc_defs.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_RTC_DEFS_H_
#define HALM_PLATFORM_NUMICRO_RTC_DEFS_H_
/*----------------------------------------------------------------------------*/
#include <xcore/bits.h>
/*------------------GPIO Control registers------------------------------------*/
#define GPIOCTL_OPMODE(pin, value)      BIT_FIELD((value), (pin) * 8)
#define GPIOCTL_OPMODE_MASK(pin)        BIT_FIELD(MASK(2), (pin) * 8)
#define GPIOCTL_OPMODE_VALUE(pin, reg) \
    FIELD_VALUE((reg), GPIOCTL_OPMODE_MASK(pin), (pin) * 8)

#define GPIOCTL_DOUT(pin)               BIT((pin) * 8 + 2)
#define GPIOCTL_CTLSEL(pin)             BIT((pin) * 8 + 3)

#define GPIOCTL_PUSEL(pin, value)       BIT_FIELD((value), (pin) * 8 + 4)
#define GPIOCTL_PUSEL_MASK(pin)         BIT_FIELD(MASK(2), (pin) * 8 + 4)
#define GPIOCTL_PUSEL_VALUE(pin, reg) \
    FIELD_VALUE((reg), GPIOCTL_PUSEL_MASK(pin), (pin) * 8 + 4)
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_RTC_DEFS_H_ */
