/*
 * platform/nxp/dac_dma.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef PLATFORM_NXP_DAC_DMA_H_
#define PLATFORM_NXP_DAC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <dma.h>
#include <interface.h>
#include <libhalm/target.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <platform/PLATFORM_TYPE/GEN_DAC/dac_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const DacDma;
/*----------------------------------------------------------------------------*/
struct DacDmaConfig
{
  /** Mandatory: conversion frequency. */
  uint32_t frequency;
  /** Mandatory: size of the single buffer in elements. */
  uint32_t size;
  /** Optional: initial output value. */
  uint16_t value;
  /** Mandatory: analog output. */
  pinNumber pin;
  /** Mandatory: memory access channel. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct DacDma
{
  struct DacBase base;

  void (*callback)(void *);
  void *callbackArgument;

  struct Dma *dma;

  /* Size of each buffer */
  uint32_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* PLATFORM_NXP_DAC_DMA_H_ */
