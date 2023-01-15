/*
 * halm/platform/numicro/adc_dma.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_ADC_DMA_H_
#define HALM_PLATFORM_NUMICRO_ADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/numicro/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDma;

struct AdcDmaConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: trigger to start the conversion. */
  enum AdcEvent event;
  /** Optional: external pin sensitivity. */
  enum PinEvent sensitivity;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
    /** Optional: sampling time extension in ADC clocks. */
  uint8_t delay;
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

  /* Output buffer */
  uint16_t *buffer;
  /* Pin descriptors */
  struct AdcPin *pins;

  /* Enabled channels mask */
  uint32_t enabled;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_ADC_DMA_H_ */
