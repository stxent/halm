/*
 * halm/platform/lpc/lpc43xx/adc_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC43XX_ADC_BASE_H_
#define HALM_PLATFORM_LPC_LPC43XX_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum [[gnu::packed]] AdcEvent
{
  ADC_BURST     = 0,
  ADC_SOFTWARE  = 1,
  ADC_CTOUT_15  = 2,
  ADC_CTOUT_8   = 3,
  ADC_TRIG0     = 4,
  ADC_TRIG1     = 5,
  ADC_MCOA2     = 6,
  ADC_EVENT_END
};

struct AdcPin
{
  /* Peripheral channel */
  uint8_t channel;
  /* Index of the control register */
  int8_t control;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

extern void adcBaseHandler0(void);
extern void adcBaseHandler1(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC43XX_ADC_BASE_H_ */
