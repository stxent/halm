/*
 * halm/platform/numicro/eadc_dma.h
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_NUMICRO_EADC_DMA_H_
#define HALM_PLATFORM_NUMICRO_EADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/numicro/eadc_base.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const EadcDma;

struct EadcDmaConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Optional: trigger delay in ADC clocks. */
  uint16_t offset;
  /** Optional: trigger to start the conversion. */
  enum AdcEvent event;
  /** Optional: external pin sensitivity. */
  enum InputEvent sensitivity;
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

struct EadcDma
{
  struct EadcBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* DMA handle */
  struct Dma *dma;

  /* Output buffer */
  uint16_t *buffer;
  /* Pin descriptors */
  struct AdcPin *pins;

  /* Sampling Module settings */
  uint32_t sampling;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NUMICRO_EADC_DMA_H_ */
