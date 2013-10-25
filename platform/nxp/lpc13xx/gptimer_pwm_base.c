/*
 * gptimer_pwm_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/gptimer_pwm.h>
/*----------------------------------------------------------------------------*/
/* Pack match channel and pin function in one value */
#define PACK_VALUE(function, channel)   (((channel) << 4) | (function))
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor gpTimerPwmPins[] = {
    {
        .key = GPIO_TO_PIN(0, 1), /* CT32B0_MAT2 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = GPIO_TO_PIN(0, 8), /* CT16B0_MAT0 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = GPIO_TO_PIN(0, 9), /* CT16B0_MAT1 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = GPIO_TO_PIN(0, 10), /* CT16B0_MAT2 */
        .channel = TIMER_CT16B0,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = GPIO_TO_PIN(0, 11), /* CT32B0_MAT3 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(3, 3)
    }, {
        .key = GPIO_TO_PIN(1, 1), /* CT32B1_MAT0 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 0)
    }, {
        .key = GPIO_TO_PIN(1, 2), /* CT32B1_MAT1 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 1)
    }, {
        .key = GPIO_TO_PIN(1, 3), /* CT32B1_MAT2 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(3, 2)
    }, {
        .key = GPIO_TO_PIN(1, 4), /* CT32B1_MAT3 */
        .channel = TIMER_CT32B1,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = GPIO_TO_PIN(1, 6), /* CT32B0_MAT0 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = GPIO_TO_PIN(1, 7), /* CT32B0_MAT1 */
        .channel = TIMER_CT32B0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = GPIO_TO_PIN(1, 9), /* CT16B1_MAT0 */
        .channel = TIMER_CT16B1,
        .value = PACK_VALUE(1, 0)
    }, {
        .key = GPIO_TO_PIN(1, 10), /* CT16B1_MAT1 */
        .channel = TIMER_CT16B1,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
