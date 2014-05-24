/*
 * platform/nxp/dac_dma.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef DAC_DMA_H_
#define DAC_DMA_H_
/*----------------------------------------------------------------------------*/
#include <interface.h>
#include <dma_list.h>
#include "dac_base.h"
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass *DacDma;
/*----------------------------------------------------------------------------*/
struct DacDmaConfig
{
  /** Mandatory: conversion frequency. */
  uint32_t frequency;
  /** Mandatory: length of the single buffer in elements. */
  uint32_t length;
  /** Optional: initial output value. */
  uint16_t value;
  /** Mandatory: analog output. */
  gpio_t pin;
  /** Mandatory: memory access channel. */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
struct DacDma
{
  struct DacBase parent;

  void (*callback)(void *);
  void *callbackArgument;

  struct DmaList *dma;

  /* Internal data buffer containing two pages and four chunks */
  uint16_t *buffer;
  /* Length of the page */
  uint32_t length;
  /* Update flag indicating that the second buffer is ready */
  bool updated;
};
/*----------------------------------------------------------------------------*/
#endif /* DAC_DMA_H_ */
