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
enum [[gnu::packed]] SctInput
{
  SCT_INPUT_NONE,
  SCT_INPUT_0,
  SCT_INPUT_1,
  SCT_INPUT_2,
  SCT_INPUT_3,
  SCT_INPUT_4,
  SCT_INPUT_5,
  SCT_INPUT_6,
  SCT_INPUT_7,
  SCT_INPUT_END
};

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
  /** Optional: edge sensitivity of the selected clock input. */
  enum InputEvent edge;
  /** Optional: clock input. */
  enum SctInput input;
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
uint8_t sctConfigInputPin(uint8_t, PinNumber, enum PinPull);
uint8_t sctConfigOutputPin(uint8_t, PinNumber, bool);
uint8_t sctGetOutputChannel(uint8_t, PinNumber);
void sctSetFrequency(struct SctBase *, uint32_t);

/* Platform-specific functions */
bool sctAllocateEvent(struct SctBase *, uint8_t *);
uint32_t sctGetClock(const struct SctBase *);
void sctReleaseEvent(struct SctBase *, uint8_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_LPC_SCT_BASE_H_ */
