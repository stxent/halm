/*
 * halm/platform/lpc/gen_1/adc_dma_stream.h
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_DMA_STREAM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_1_ADC_DMA_STREAM_H_
#define HALM_PLATFORM_LPC_GEN_1_ADC_DMA_STREAM_H_
/*----------------------------------------------------------------------------*/
#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/gpdma_base.h>
#include <xcore/stream.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const AdcDmaStream;

struct AdcDmaStreamHandler;

struct AdcDmaStreamConfig
{
  /** Mandatory: pointer to an array of pins terminated by a zero element. */
  const PinNumber *pins;
  /** Mandatory: request queue size. */
  size_t size;
  /** Optional: desired converter clock. */
  uint32_t frequency;

  struct
  {
    /** Optional: trigger to start the conversion. */
    enum AdcEvent event;
    /** Mandatory: DMA channel. */
    uint8_t dma;
  } converter;

  struct
  {
    /** Mandatory: DMA event. */
    enum GpDmaEvent event;
    /** Mandatory: DMA channel. */
    uint8_t dma;
  } memory;

  /** Optional: number of bits of accuracy of the result. */
  uint8_t accuracy;
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: disable automatic peripheral locking. */
  bool shared;
};

struct AdcDmaStream
{
  struct AdcBase base;

  /* Input stream */
  struct AdcDmaStreamHandler *stream;
  /* Inner DMA handle */
  struct Dma *inner;
  /* Outer DMA handle */
  struct Dma *outer;

  /* Pin descriptors */
  struct AdcPin pins[8];
  /* Event mask */
  uint32_t mask;
  /* Temporary buffer */
  uint16_t buffer;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Stream *adcDmaStreamGetInput(struct AdcDmaStream *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_1_ADC_DMA_STREAM_H_ */
