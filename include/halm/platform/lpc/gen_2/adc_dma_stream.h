/*
 * halm/platform/lpc/gen_2/adc_dma_stream.h
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_ADC_DMA_STREAM_H_
#error This header should not be included directly
#endif

#ifndef HALM_PLATFORM_LPC_GEN_2_ADC_DMA_STREAM_H_
#define HALM_PLATFORM_LPC_GEN_2_ADC_DMA_STREAM_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/lpc/adc_base.h>
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

struct AdcDmaStream
{
  struct AdcBase base;

  /* Input stream */
  struct AdcDmaStreamHandler *stream;
  /* DMA handle */
  struct Dma *dma;

  /* Pin descriptors */
  struct AdcPin *pins;

  /* Sequence control register value */
  uint32_t control;
  /* Pin count */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Stream *adcDmaStreamGetInput(struct AdcDmaStream *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_GEN_2_ADC_DMA_STREAM_H_ */
