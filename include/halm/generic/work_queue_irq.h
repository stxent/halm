/*
 * halm/generic/work_queue_irq.h
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_GENERIC_WORK_QUEUE_IRQ_H_
#define HALM_GENERIC_WORK_QUEUE_IRQ_H_
/*----------------------------------------------------------------------------*/
#include <halm/irq.h>
#include <halm/wq.h>
/*----------------------------------------------------------------------------*/
extern const struct WorkQueueClass * const WorkQueueIrq;

struct WorkQueueIrqConfig
{
  /** Mandatory: number of queued tasks. */
  size_t size;
  /** Mandatory: number of an interrupt vector. */
  IrqNumber irq;
  /** Optional: interrupt priority. */
  IrqPriority priority;
};
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

void wqIrqUpdate(void *);

END_DECLS
/*----------------------------------------------------------------------------*/
#define DEFINE_WQ_IRQ(name) \
    extern void *name;

#define DECLARE_WQ_IRQ(name, isr) \
    void *name = NULL; \
    \
    void isr(void) \
    { \
      wqIrqUpdate(name); \
    }
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_WORK_QUEUE_IRQ_H_ */
