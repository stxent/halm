/*
 * halm/platform/lpc/sct_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_PLATFORM_LPC_SCT_BASE_H_
#define HALM_PLATFORM_LPC_SCT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
#undef HEADER_PATH
#define HEADER_PATH <halm/platform/PLATFORM_TYPE/PLATFORM/sct_base.h>
#include HEADER_PATH
#undef HEADER_PATH
/*----------------------------------------------------------------------------*/
enum [[gnu::packed]] SctPart
{
  SCT_LOW,
  SCT_HIGH,
  SCT_UNIFIED
};
/*----------------------------------------------------------------------------*/
extern const struct EntityClass * const SctBase;

struct SctBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Mandatory: timer part. */
  enum SctPart part;
};

struct SctBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  IrqNumber irq;

  /* Events that may cause interrupt request */
  uint16_t mask;
  /* Unique peripheral identifier */
  uint8_t channel;
  /* Timer configuration */
  enum SctPart part;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

/* Common functions */
void sctSetFrequency(struct SctBase *, uint32_t);

/* Platform-specific functions */
bool sctAllocateEvent(struct SctBase *, uint8_t *);
enum SctInput sctAllocateInputChannel(struct SctBase *, PinNumber);
enum SctOutput sctAllocateOutputChannel(struct SctBase *, PinNumber);
void sctConfigInputPin(struct SctBase *, enum SctInput, PinNumber,
    enum PinPull);
void sctConfigOutputPin(struct SctBase *, enum SctOutput, PinNumber, bool);
uint32_t sctGetClock(const struct SctBase *);
void sctReleaseEvent(struct SctBase *, uint8_t);
void sctReleaseInputChannel(struct SctBase *, enum SctInput);
void sctReleaseOutputChannel(struct SctBase *, enum SctOutput);
bool sctReserveInputChannel(struct SctBase *, enum SctInput);
bool sctReserveOutputChannel(struct SctBase *, enum SctOutput);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SCT_BASE_H_ */
