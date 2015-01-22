/*
 * platform/nxp/adc_dma.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_ADC_DMA_H_
#define PLATFORM_NXP_ADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <dma.h>
#include <interface.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GENERATION/adc_unit.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDma;
/*----------------------------------------------------------------------------*/
struct AdcDmaConfig
{
  /** Mandatory: peripheral unit. */
  struct AdcUnit *parent;
  /** Mandatory: size of the single buffer in elements. */
  uint32_t size;
  /** Mandatory: analog input. */
  pin_t pin;
  /** Mandatory: memory access channel. */
  uint8_t channel;
  /** Optional: trigger to start the conversion. */
  enum adcEvent event;
};
/*----------------------------------------------------------------------------*/
struct AdcDma
{
  struct Entity parent;

  /* Pointer to a parent unit */
  struct AdcUnit *unit;

  void (*callback)(void *);
  void *callbackArgument;

  /* Direct memory access channel descriptor */
  struct Dma *dma;
  /* Size of each buffer */
  uint32_t size;

  /* Pin descriptor */
  struct AdcPin pin;
  /* Hardware trigger event */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_ADC_DMA_H_ */
