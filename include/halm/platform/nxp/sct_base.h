/*
 * halm/platform/nxp/sct_base.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef HALM_PLATFORM_NXP_SCT_BASE_H_
#define HALM_PLATFORM_NXP_SCT_BASE_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/pin.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
enum SctInput
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

enum SctPart
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
  enum PinEvent edge;
  /** Optional: clock input. */
  enum SctInput input;
  /** Optional: timer part. */
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
uint32_t sctGetClock(const struct SctBase *);
void sctReleaseEvent(struct SctBase *, uint8_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SCT_BASE_H_ */
