/*
 * halm/platform/lpc/gen_2/adc_dma.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_DMA_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_ADC_DMA_H_
#define HALM_PLATFORM_LPC_GEN_2_ADC_DMA_H_
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
  /** Mandatory: converter sequence identifier. */
  enum AdcSequence sequence;
  /** Optional: input sensitivity. */
  enum InputEvent sensitivity;
  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Mandatory: DMA channel. */
  uint8_t dma;
  /** Optional: DMA priority. Low priority is used by default. */
  uint8_t priority;
  /** Optional: enable sequence preemption, available for sequence A only. */
  bool preemption;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
  /** Optional: use one trigger for one conversion. */
  bool singlestep;
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

  /* Sequence control register value */
  uint32_t control;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_ADC_DMA_H_ */
