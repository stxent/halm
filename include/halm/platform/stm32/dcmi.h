/*
 * halm/platform/stm32/dcmi.h
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_STM32_DCMI_H_
#define HALM_PLATFORM_STM32_DCMI_H_
/*----------------------------------------------------------------------------*/
#include <halm/dma.h>
#include <halm/platform/stm32/dcmi_base.h>
#include <xcore/interface.h>
#include <xcore/stream.h>
/*----------------------------------------------------------------------------*/
extern const struct InterfaceClass * const Dcmi;

/** Frame capture rate. */
enum [[gnu::packed]] DcmiCaptureRate
{
  /** Capture all frames. */
  DCMI_ALL_FRAME,
  /** Capture every second frame. */
  DCMI_ALT_2_FRAME,
  /** Capture every fourth frame. */
  DCMI_ALT_4_FRAME
};

/** DCMI driver parameters. */
enum DcmiParameter
{
  /**
   * Enable or disable snapshot mode. Parameter type is \p uint8_t.
   * Valid values: 0 disables snapshot mode, 1 enables snapshot mode.
   */
  IF_DCMI_SNAPSHOT,
  /**
   * Get or set the JPEG mode flag. Parameter type is \p uint8_t.
   * Valid values: 0 disables JPEG mode, 1 enables JPEG mode.
   */
  IF_DCMI_JPEG
};

struct DcmiConfig
{
  /** Mandatory: number of DMA buffers in the capture queue. */
  size_t size;

  /** Optional: interrupt priority. */
  IrqPriority irqPriority;

  /** Optional: frame capture rate. All frames are captured by default. */
  enum DcmiCaptureRate rate;
  /** Optional: DMA priority. Low priority is used by default. */
  enum DmaPriority dmaPriority;

  /** Mandatory: DMA stream number. */
  uint8_t dma;
  /** Mandatory: data bus width in bits (8, 10, 12 or 14). */
  uint8_t width;

  /** Optional: @b false for snapshot mode, @b true for continuous mode. */
  bool continuous;
  /** Optional: enable JPEG mode. */
  bool jpeg;

  /** Camera interface pins. */
  struct
  {
    /** Mandatory: array of data lines terminated by a zero element. */
    const PinNumber *data;
    /** Optional: horizontal synchronization line. */
    PinNumber hsync;
    /** Optional: vertical synchronization line. */
    PinNumber vsync;
    /** Mandatory: pixel clock line. */
    PinNumber pixclk;
  } pins;

  /** Signal polarities. */
  struct
  {
    /**
     * Mandatory: active high horizontal synchronization. Indicates
     * the pin level when data is not valid on the parallel interface.
     */
    bool hsync;
    /**
     * Mandatory: active high vertical synchronization. Indicates
     * the pin level when data is not valid on the parallel interface.
     */
    bool vsync;
    /** Mandatory: capture on rising edge of the pixel clock. */
    bool pixclk;
  } polarity;
};

struct Dcmi
{
  struct DcmiBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Input stream */
  struct DcmiStream *stream;
  /* DMA list stream descriptor */
  struct Dma *dma;

  /* Continuous capture mode */
  bool continuous;
  /* Error flag */
  bool error;
  /* JPEG mode enabled */
  bool jpeg;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

struct Stream *dcmiGetInput(struct Dcmi *);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_STM32_DCMI_H_ */
