/*
 * halm/platform/lpc/adc_dma.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_DMA_H_
#define HALM_PLATFORM_LPC_ADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDma;

struct AdcDmaConfig
{
  /** Optional: desired converter clock. */
  uint32_t frequency;
  /** Optional: trigger to start the conversion. */
  enum AdcEvent event;
  /** Mandatory: analog input. */
  PinNumber pin;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: DMA channel. */
  uint8_t dma;
};

struct AdcDma
{
  struct AdcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA channel handle */
  struct Dma *dma;
  /* Pin descriptor */
  struct AdcPin pin;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_ADC_DMA_H_ */
