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
extern const struct EntityClass * const SctBase;
/*----------------------------------------------------------------------------*/
enum sctInput
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
/*----------------------------------------------------------------------------*/
enum sctPart
{
  SCT_UNIFIED,
  SCT_LOW,
  SCT_HIGH
};
/*----------------------------------------------------------------------------*/
struct SctBaseConfig
{
  /** Mandatory: peripheral identifier. */
  uint8_t channel;
  /** Optional: edge sensitivity of the selected clock input. */
  enum pinEvent edge;
  /** Optional: clock input. */
  enum sctInput input;
  /** Optional: timer part. */
  enum sctPart part;
};
/*----------------------------------------------------------------------------*/
struct SctBase
{
  struct Timer base;

  void *reg;
  void (*handler)(void *);
  irqNumber irq;

  /* Events that may cause interrupt request */
  uint16_t mask;
  /* Unique peripheral identifier */
  uint8_t channel;
  /* Timer configuration */
  enum sctPart part;
};
/*----------------------------------------------------------------------------*/
int sctAllocateEvent(struct SctBase *);
uint32_t sctGetClock(const struct SctBase *);
void sctReleaseEvent(struct SctBase *, int);
/*----------------------------------------------------------------------------*/
#endif /* HALM_PLATFORM_NXP_SCT_BASE_H_ */
