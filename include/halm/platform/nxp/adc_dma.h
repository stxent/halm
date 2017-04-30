/*
 * halm/platform/nxp/adc_dma.h
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_ADC_DMA_H_
#define HALM_PLATFORM_NXP_ADC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <xcore/interface.h>
#include <halm/dma.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_ADC/adc_unit.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDma;
/*----------------------------------------------------------------------------*/
struct AdcDmaConfig
{
  /** Mandatory: peripheral unit. */
  struct AdcUnit *parent;
  /** Optional: trigger to start the conversion. */
  enum AdcEvent event;
  /** Mandatory: analog input. */
  PinNumber pin;
  /** Mandatory: memory access channel. */
  uint8_t dma;
};
/*----------------------------------------------------------------------------*/
struct AdcDma
{
  struct Interface base;

  /* Pointer to a parent unit */
  struct AdcUnit *unit;

  void (*callback)(void *);
  void *callbackArgument;

  /* Direct memory access channel descriptor */
  struct Dma *dma;

  /* Pin descriptor */
  struct AdcPin pin;
  /* Hardware trigger event */
  uint8_t event;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_ADC_DMA_H_ */
