/*
 * halm/platform/lpc/adc_dma.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_DMA_H_
#define HALM_PLATFORM_LPC_ADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/lpc/adc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDma;

struct AdcDmaConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: desired converter clock. */
  uint32_t frequency;
  /** Optional: trigger to start the conversion. */
  enum AdcEvent event;
  /** Optional: number of bits of accuracy of the result. */
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

  /* Pin descriptors */
  struct AdcPin pins[8];
  /* Output buffer */
  uint16_t buffer[8];

  /* Event mask */
  uint32_t mask;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_ADC_DMA_H_ */
