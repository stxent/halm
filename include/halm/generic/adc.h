/*
 * halm/generic/adc.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_ADC_H_
#define HALM_GENERIC_ADC_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
/*----------------------------------------------------------------------------*/
enum ADCParameter
{
  /** Start calibration process. Data pointer should be set to zero. */
  IF_ADC_CALIBRATE = IF_PARAMETER_END
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_ADC_H_ */
