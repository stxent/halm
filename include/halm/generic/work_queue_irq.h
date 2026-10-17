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

#define DECLARE_WQ_IRQ_IMPL(name, isr, irq, attribute) \
    void *name = nullptr; \
    \
    attribute void isr(void) \
    { \
      if (irq != IRQ_RESERVED) \
        irqClearPending(irq); \
      wqIrqUpdate(name); \
    }

#define DECLARE_WQ_IRQ_2(name, isr) \
    DECLARE_WQ_IRQ_IMPL(name, isr, IRQ_RESERVED,)

#define DECLARE_WQ_IRQ_3(name, isr, irq) \
    DECLARE_WQ_IRQ_IMPL(name, isr, irq,)

#define DECLARE_WQ_IRQ_4(name, isr, irq, attribute) \
    DECLARE_WQ_IRQ_IMPL(name, isr, irq, attribute)

/* Argument counter and dispatcher mechanics */
#define WQ_GET_MACRO_HELPER(_1, _2, _3, _4, NAME, ...) NAME

/* Universal public interface */
#define DECLARE_WQ_IRQ(...) \
    WQ_GET_MACRO_HELPER(__VA_ARGS__, \
        DECLARE_WQ_IRQ_4, DECLARE_WQ_IRQ_3, DECLARE_WQ_IRQ_2)(__VA_ARGS__)
/*----------------------------------------------------------------------------*/
#endif /* HALM_GENERIC_WORK_QUEUE_IRQ_H_ */
