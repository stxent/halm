/*
 * halm/platform/stm32/adc_base.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_BASE_H_
#define HALM_PLATFORM_STM32_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/pin.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
struct AdcPin adcConfigPin(const struct AdcBase *, PinNumber);
uint32_t adcEncodeSensitivity(enum InputEvent);
void adcReleasePin(struct AdcPin);
void adcSetupPins(struct AdcBase *, const PinNumber *, struct AdcPin *, size_t);

/* Platform-specific functions */
void *adcMakeCircularDma(uint8_t, uint8_t, enum DmaPriority, bool);
struct AdcBase *adcGetInstance(uint8_t);
bool adcSetInstance(uint8_t, struct AdcBase *, struct AdcBase *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_ADC_BASE_H_ */
