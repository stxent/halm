/*
 * exti_base.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/platform_defs.h>
#include <halm/platform/stm32/exti_base.h>
#include <halm/platform/stm32/exti_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void enableInterrupt(enum ExtiEvent, IrqPriority);
static IrqNumber eventToIrq(enum ExtiEvent);
static bool setInstance(uint8_t, struct ExtiBase *);

#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static void disableInterrupt(enum ExtiEvent);
static bool isEventGroupUsed(enum ExtiEvent);
static bool isGroupUsed(size_t, size_t);
#endif
/*----------------------------------------------------------------------------*/
static enum Result extiInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static void extiDeinit(void *);
#else
#define extiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const ExtiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = extiInit,
    .deinit = extiDeinit
};
/*----------------------------------------------------------------------------*/
static struct ExtiBase *instances[32] = {0};
/*----------------------------------------------------------------------------*/
static void enableInterrupt(enum ExtiEvent event, IrqPriority priority)
{
  const IrqNumber irq = eventToIrq(event);

  if (irq != IRQ_RESERVED)
  {
    if (irqGetPriority(irq) < priority)
      irqSetPriority(irq, priority);

    irqEnable(irq);
  }
}
/*----------------------------------------------------------------------------*/
static IrqNumber eventToIrq(enum ExtiEvent event)
{
  if (event <= EXTI_PIN1)
  {
    return EXTI0_1_IRQ;
  }
  else if (event >= EXTI_PIN2 && event <= EXTI_PIN3)
  {
    return EXTI2_3_IRQ;
  }
  else if (event >= EXTI_PIN4 && event <= EXTI_PIN15)
  {
    return EXTI4_15_IRQ;
  }

  switch (event)
  {
    case EXTI_USB_WAKEUP:
      return USB_IRQ;

    case EXTI_COMPARATOR1:
    case EXTI_COMPARATOR2:
      return ADC1_COMP_IRQ;

    case EXTI_I2C1_WAKEUP:
      return I2C1_IRQ;

    case EXTI_USART1_WAKEUP:
      return USART1_IRQ;

    case EXTI_USART2_WAKEUP:
      return USART2_IRQ;

    case EXTI_CEC_WAKEUP:
      return CEC_CAN_IRQ;

    case EXTI_USART3_WAKEUP:
      return USART3_8_IRQ;

    default:
      return IRQ_RESERVED;
  }
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct ExtiBase *object)
{
  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static void disableInterrupt(enum ExtiEvent event)
{
  if (!isEventGroupUsed(event))
  {
    const IrqNumber irq = eventToIrq(event);

    if (irq != IRQ_RESERVED)
      irqDisable(irq);
  }
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static bool isEventGroupUsed(enum ExtiEvent event)
{
  assert(event < EXTI_EVENT_END);

  if (event <= EXTI_PIN1)
    return isGroupUsed(EXTI_PIN0, EXTI_PIN1);
  else if (event >= EXTI_PIN2 && event <= EXTI_PIN3)
    return isGroupUsed(EXTI_PIN2, EXTI_PIN3);
  else if (event >= EXTI_PIN4 && event <= EXTI_PIN15)
    return isGroupUsed(EXTI_PIN4, EXTI_PIN15);
  else
    return instances[event] != NULL;
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static bool isGroupUsed(size_t begin, size_t end)
{
  for (size_t index = begin; index <= end; ++index)
  {
    if (instances[index])
      return true;
  }

  return false;
}
#endif
/*----------------------------------------------------------------------------*/
void EXTI0_1_ISR(void)
{
  const uint32_t pr = STM_EXTI->PR;
  STM_EXTI->PR = pr & PR_PIF_MASK(2, 0);

  if (pr & PR_PIF(0))
    instances[0]->handler(instances[0]);
  if (pr & PR_PIF(1))
    instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void EXTI2_3_ISR(void)
{
  const uint32_t pr = STM_EXTI->PR;
  STM_EXTI->PR = pr & PR_PIF_MASK(4, 2);

  if (pr & PR_PIF(2))
    instances[2]->handler(instances[2]);
  if (pr & PR_PIF(3))
    instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void EXTI4_15_ISR(void)
{
  uint32_t pr = STM_EXTI->PR & PR_PIF_MASK(16, 4);
  STM_EXTI->PR = pr;

  do
  {
    const unsigned int index = 31 - countLeadingZeros32(pr);

    instances[index]->handler(instances[index]);
    pr -= 1UL << index;
  }
  while (pr);
}
/*----------------------------------------------------------------------------*/
static enum Result extiInit(void *object, const void *configBase)
{
  const struct ExtiBaseConfig * const config = configBase;
  struct ExtiBase * const interrupt = object;

  if (!setInstance(config->channel, interrupt))
    return E_BUSY;

  interrupt->channel = config->channel;
  interrupt->handler = NULL;

  if (!sysClockStatus(CLK_SYSCFG))
  {
    sysClockEnable(CLK_SYSCFG);
    sysResetEnable(RST_SYSCFG);
    sysResetDisable(RST_SYSCFG);
  }

  /* Configure event multiplexor */
  if (interrupt->channel <= EXTI_PIN15)
  {
    const size_t index = interrupt->channel >> 2;
    const size_t offset = interrupt->channel & 0x03;
    uint32_t exticr = STM_SYSCFG->EXTICR[index];

    exticr &= ~EXTICR_INPUT_MASK(offset);
    exticr |= EXTICR_INPUT(offset, PIN_TO_PORT(config->pin));

    STM_SYSCFG->EXTICR[index] = exticr;
  }

  /* Mask interrupt and enable it in the NVIC */
  STM_EXTI->IMR &= ~IMR_IM(interrupt->channel);
  enableInterrupt(interrupt->channel, config->priority);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static void extiDeinit(void *object)
{
  const struct ExtiBase * const interrupt = object;

  instances[interrupt->channel] = NULL;
  disableInterrupt(interrupt->channel);
}
#endif
