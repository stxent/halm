/*
 * halm/platform/lpc/lpc82x/adc_base.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_BASE_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_LPC82X_ADC_BASE_H_
#define HALM_PLATFORM_LPC_LPC82X_ADC_BASE_H_
/*----------------------------------------------------------------------------*/
#include <xcore/helpers.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
/* ADC trigger sources */
enum [[gnu::packed]] AdcEvent
{
  ADC_SOFTWARE  = 0,
  ADC_PINTRIG0  = 1,
  ADC_PINTRIG1  = 2,
  ADC_SCT0_OUT3 = 3,
  ADC_ACMP_O    = 4,
  ADC_ARM_TXEV  = 5,

  ADC_BURST,
  ADC_EVENT_END
};

enum [[gnu::packed]] AdcSequence
{
  ADC0_SEQA,
  ADC0_SEQB,
  ADC_SEQ_END
};

struct AdcPin
{
  /* Conversion channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

extern void adcBaseHandler0A(void);
extern void adcBaseHandler0B(void);
extern void adcBaseOverrunHandler0(void);
extern void adcBaseThresholdHandler0(void);

END_DECLS
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_LPC82X_ADC_BASE_H_ */
