/*
 * halm/platform/nxp/dac_dma.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_DAC_DMA_H_
#define HALM_PLATFORM_NXP_DAC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <xcore/interface.h>
#include <halm/dma.h>
#include <halm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/GEN_DAC/dac_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const DacDma;

struct DacDmaConfig
{
  /** Mandatory: conversion rate. */
  uint32_t rate;
  /** Optional: initial output value. */
  uint16_t value;
  /** Mandatory: analog output. */
  PinNumber pin;
  /** Mandatory: memory access channel. */
  uint8_t dma;
};

struct DacDma
{
  struct DacBase base;

  void (*callback)(void *);
  void *callbackArgument;

  struct Dma *dma;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_DAC_DMA_H_ */
