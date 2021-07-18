/*
 * halm/platform/lpc/dac_dma.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_DAC_DMA_H_
#define HALM_PLATFORM_LPC_DAC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/lpc/dac_base.h>
#include <xcore/stream.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const DacDma;

struct DacDmaStream;

struct DacDmaConfig
{
  /** Mandatory: request queue size. */
  size_t size;
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

  /* Output stream */
  struct DacDmaStream *stream;
  /* DMA channel descriptors for transmitting data */
  struct Dma *dma;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Stream *dacDmaGetOutput(struct DacDma *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_DAC_DMA_H_ */
