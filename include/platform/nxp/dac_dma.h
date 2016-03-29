/*
 * platform/nxp/dac_dma.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_DAC_DMA_H_
#define HALM_PLATFORM_NXP_DAC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <dma.h>
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
  /** Mandatory: conversion rate. */
  uint32_t rate;
  /** Mandatory: size in elements of each buffer. */
  uint16_t size;
  /** Optional: initial output value. */
  uint16_t value;
  /** Mandatory: analog output. */
  pinNumber pin;
  /** Mandatory: memory access channel. */
  uint8_t dma;
};
/*----------------------------------------------------------------------------*/
struct DacDma
{
  struct DacBase base;

  void (*callback)(void *);
  void *callbackArgument;

  struct Dma *dma;

  /* Size of each buffer */
  uint16_t size;
};
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_DAC_DMA_H_ */
