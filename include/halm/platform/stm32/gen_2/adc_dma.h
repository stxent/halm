/*
 * halm/platform/stm32/gen_2/adc_dma.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_ADC_DMA_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_STM32_GEN_2_ADC_DMA_H_
#define HALM_PLATFORM_STM32_GEN_2_ADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/stm32/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDma;

struct AdcDmaConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: trigger to start the conversion, regular group is used. */
  enum AdcEvent event;
  /** Optional: trigger sensitivity when external trigger is used. */
  enum InputEvent sensitivity;
  /** Optional: sampling time of a single channel. */
  enum AdcSamplingTime time;
  /** Optional: resolution of the conveserion. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: DMA channel. */
  uint8_t dma;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct AdcDma
{
  struct AdcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA handle */
  struct Dma *dma;

  /* Precalculated value ot the Configuration Register 1 */
  uint32_t config;
  /* Output buffer */
  uint16_t *buffer;
  /* Pin descriptors */
  struct AdcPin *pins;

  /* Sampling time */
  enum AdcSamplingTime time;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_GEN_2_ADC_DMA_H_ */
