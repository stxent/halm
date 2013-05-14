/*
 * base_timer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "base_timer.h"
#include "base_timer_defs.h"
#include "lpc13xx_sys.h"
#include "mutex.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV_VALUE   1
#define DEFAULT_PRIORITY    15 /* Lowest interrupt priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
///* Timer pin function values */
//static const struct GpioPinFunc timerPins[] = {
//    {
//        .key = GPIO_TO_PIN(1, 6),
//        .func = 1
//    },
//    {
//        .key = GPIO_TO_PIN(1, 7),
//        .func = 1
//    },
//    {} /* End of pin function association list */
//};
/*----------------------------------------------------------------------------*/
static enum result btInit(void *, const void *);
static void btDeinit(void *);
static void btHandler(void *);
static void btSetFrequency(void *, uint32_t);
static void btSetHandler(void *, void (*)(void *), void *);
static void btSetOverflow(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct TimerClass timerTable = {
    .size = sizeof(struct BaseTimer),
    .init = btInit,
    .deinit = btDeinit,

    .setFrequency = btSetFrequency,
    .setOverflow = btSetOverflow,
    .setHandler = btSetHandler,
    .handler = btHandler,

    .Capture = 0,
    .Pwm = 0,

    .createCapture = 0,
    .createPwm = 0
};
/*----------------------------------------------------------------------------*/
const struct TimerClass *BaseTimer = &timerTable;
/*----------------------------------------------------------------------------*/
static void * volatile descriptors[] = {0, 0, 0, 0};
static Mutex lock = MUTEX_UNLOCKED;
/*----------------------------------------------------------------------------*/
static void btHandler(void *object)
{
  struct BaseTimer *device = object;

  if (device->reg->IR & IR_MATCH_INTERRUPT(0)) /* Match 0 */
  {
    if (device->handler)
      device->handler(device->handlerParameters);
    device->reg->IR = IR_MATCH_INTERRUPT(0); /* Clear flag */
  }
}
/*----------------------------------------------------------------------------*/
enum result btSetDescriptor(uint8_t channel, void *descriptor)
{
  enum result res = E_ERROR;

  assert(channel < sizeof(descriptors));

  mutexLock(&lock);
  if (!descriptors[channel])
  {
    descriptors[channel] = descriptor;
    res = E_OK;
  }
  mutexUnlock(&lock);
  return res;
}
/*----------------------------------------------------------------------------*/
void TIMER16_0_IRQHandler(void)
{
  if (descriptors[0])
    ((struct TimerClass *)CLASS(descriptors[0]))->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void TIMER16_1_IRQHandler(void)
{
  if (descriptors[1])
    ((struct TimerClass *)CLASS(descriptors[1]))->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void TIMER32_0_IRQHandler(void)
{
  if (descriptors[2])
    ((struct TimerClass *)CLASS(descriptors[2]))->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void TIMER32_1_IRQHandler(void)
{
  if (descriptors[3])
    ((struct TimerClass *)CLASS(descriptors[3]))->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
static void btSetFrequency(void *object, uint32_t frequency)
{

}
/*----------------------------------------------------------------------------*/
static void btSetOverflow(void *object, uint32_t overflow)
{
  struct BaseTimer *device = object;

  device->reg->MR0 = overflow;
  /* Synchronously reset prescaler and counter registers */
  device->reg->TCR |= TCR_CRES;
  while (device->reg->TC || device->reg->PC);
  device->reg->TCR &= ~TCR_CRES;
}
/*----------------------------------------------------------------------------*/
static void btSetHandler(void *object, void (*handler)(void *),
    void *parameters)
{
  struct BaseTimer *device = object;

  device->handler = handler;
  device->handlerParameters = parameters;
  if (handler)
  {
    /* Enable Match 0 interrupt, reset counter after each interrupt */
    device->reg->MCR |= MCR_INTERRUPT(0) | MCR_RESET(0);
  }
  else
  {
    /* Disable Match 0 interrupt */
    device->reg->MCR &= ~(MCR_INTERRUPT(0) | MCR_RESET(0));
  }
}
/*----------------------------------------------------------------------------*/
static void btDeinit(void *object)
{
  struct BaseTimer *device = object;

  /* Disable interrupt */
  NVIC_DisableIRQ(device->irq);
  /* Disable Timer clock */
  switch (device->channel)
  {
    case 0:
      sysClockDisable(CLK_CT16B0);
      break;
    case 1:
      sysClockDisable(CLK_CT16B1);
      break;
    case 2:
      sysClockDisable(CLK_CT32B0);
      break;
    case 3:
      sysClockDisable(CLK_CT32B1);
      break;
  }
//  /* Release pins */
//  gpioDeinit(&device->txPin);
//  gpioDeinit(&device->rxPin);
  /* Reset Timer descriptor */
  btSetDescriptor(device->channel, 0);
}
/*----------------------------------------------------------------------------*/
static enum result btInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct BaseTimerConfig *config = configPtr;
  struct BaseTimer *device = object;

  /* Check device configuration data */
  assert(config);

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if (btSetDescriptor(device->channel, device) != E_OK)
    return E_ERROR;

  switch (device->channel)
  {
    case 0:
      sysClockEnable(CLK_CT16B0);
      device->reg = LPC_TMR16B0;
      device->irq = TIMER_16_0_IRQn;
      break;
    case 1:
      sysClockEnable(CLK_CT16B1);
      device->reg = LPC_TMR16B1;
      device->irq = TIMER_16_1_IRQn;
      break;
    case 2:
      sysClockEnable(CLK_CT32B0);
      device->reg = LPC_TMR32B0;
      device->irq = TIMER_32_0_IRQn;
      break;
    case 3:
      sysClockEnable(CLK_CT32B1);
      device->reg = LPC_TMR32B1;
      device->irq = TIMER_32_1_IRQn;
      break;
  }

  device->reg->PR = (SystemCoreClock / DEFAULT_DIV_VALUE)
      / config->frequency - 1;
  /* Reset control registers */
  device->reg->MCR = 0;
//  device->reg->CCR = 0;
  /* Reset internal counters */
  device->reg->PC = 0;
  device->reg->TC = 0;
  /* Enable timer/counter */
  device->reg->TCR = TCR_CEN;

  /* Clear pending interrupts */
  device->reg->IR = 0x1F;
  /* Enable interrupt */
  NVIC_EnableIRQ(device->irq);
  /* Set interrupt priority, lowest by default */
  NVIC_SetPriority(device->irq, DEFAULT_PRIORITY);
  return E_OK;
}
