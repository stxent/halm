/*
 * exti_base.c
 * Copyright (C) 2024 xent
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
#  define extiDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const ExtiBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = extiInit,
    .deinit = extiDeinit
};
/*----------------------------------------------------------------------------*/
static struct ExtiBase *instances[23] = {NULL};
/*----------------------------------------------------------------------------*/
static void enableInterrupt(enum ExtiEvent event, IrqPriority priority)
{
  const IrqNumber irq = eventToIrq(event);

  if (irqGetPriority(irq) < priority)
    irqSetPriority(irq, priority);

  irqEnable(irq);
}
/*----------------------------------------------------------------------------*/
static IrqNumber eventToIrq(enum ExtiEvent event)
{
  if (event <= EXTI_PIN4)
  {
    return EXTI0_IRQ + (event - EXTI_PIN0);
  }
  else if (event <= EXTI_PIN9)
  {
    return EXTI9_5_IRQ;
  }
  else if (event <= EXTI_PIN15)
  {
    return EXTI15_10_IRQ;
  }

  switch (event)
  {
    case EXTI_PVD:
      return PVD_IRQ;

    case EXTI_RTC_ALARM:
      return RTC_ALARM_IRQ;

    case EXTI_USB_FS_WAKEUP:
      return OTG_FS_WKUP_IRQ;

    case EXTI_ETHERNET_WAKEUP:
      return ETH_WKUP_IRQ;

    case EXTI_USB_HS_WAKEUP:
      return OTG_HS_WKUP_IRQ;

    case EXTI_RTC_TAMPER_TIMESTAMP:
      return TAMP_STAMP_IRQ;

    case EXTI_RTC_WAKEUP:
      return RTC_WKUP_IRQ;

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
    irqDisable(eventToIrq(event));
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_EXTI_NO_DEINIT
static bool isEventGroupUsed(enum ExtiEvent event)
{
  assert(event < EXTI_EVENT_END);

  if (event >= EXTI_PIN5 && event <= EXTI_PIN9)
    return isGroupUsed(EXTI_PIN5, EXTI_PIN9);
  else if (event >= EXTI_PIN10 && event <= EXTI_PIN15)
    return isGroupUsed(EXTI_PIN10, EXTI_PIN15);
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
    if (instances[index] != NULL)
      return true;
  }

  return false;
}
#endif
/*----------------------------------------------------------------------------*/
void EXTI0_ISR(void)
{
  STM_EXTI->PR = PR_PIF(0);
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void EXTI1_ISR(void)
{
  STM_EXTI->PR = PR_PIF(1);
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void EXTI2_ISR(void)
{
  STM_EXTI->PR = PR_PIF(2);
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void EXTI3_ISR(void)
{
  STM_EXTI->PR = PR_PIF(3);
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void EXTI4_ISR(void)
{
  STM_EXTI->PR = PR_PIF(4);
  instances[4]->handler(instances[4]);
}
/*----------------------------------------------------------------------------*/
void EXTI9_5_ISR(void)
{
  uint32_t pr = STM_EXTI->PR & PR_PIF_MASK(10, 5);
  STM_EXTI->PR = pr;

  pr = reverseBits32(pr);

  do
  {
    const unsigned int index = countLeadingZeros32(pr);

    instances[index]->handler(instances[index]);
    pr -= (1UL << 31) >> index;
  }
  while (pr);
}
/*----------------------------------------------------------------------------*/
void EXTI15_10_ISR(void)
{
  uint32_t pr = STM_EXTI->PR & PR_PIF_MASK(16, 10);
  STM_EXTI->PR = pr;

  pr = reverseBits32(pr);

  do
  {
    const unsigned int index = countLeadingZeros32(pr);

    instances[index]->handler(instances[index]);
    pr -= (1UL << 31) >> index;
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

  /* Configure event multiplexor */
  if (interrupt->channel <= EXTI_PIN15)
  {
    if (!sysClockStatus(CLK_SYSCFG))
      sysClockEnable(CLK_SYSCFG);

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
