/*
 * base_timer.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/system.h"
#include "platform/nxp/gptimer_defs.h"
#include "platform/nxp/lpc13xx/gptimer.h"
#include "platform/nxp/lpc13xx/interrupts.h"
#include "platform/nxp/lpc13xx/power.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV_VALUE 1
#define DEFAULT_PRIORITY  255 /* Lowest interrupt priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
static enum result setDescriptor(uint8_t, struct GpTimer *);
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *, const void *);
static void tmrDeinit(void *);
static void tmrCallback(void *, void (*)(void *), void *);
static void tmrControl(void *, bool);
static void tmrSetFrequency(void *, uint32_t);
static void tmrSetOverflow(void *, uint32_t);
static uint32_t tmrValue(void *);
/*----------------------------------------------------------------------------*/
static const struct TimerClass timerTable = {
    .size = sizeof(struct GpTimer),
    .init = tmrInit,
    .deinit = tmrDeinit,

    .callback = tmrCallback,
    .control = tmrControl,
    .setFrequency = tmrSetFrequency,
    .setOverflow = tmrSetOverflow,
    .value = tmrValue
};
/*----------------------------------------------------------------------------*/
const struct TimerClass *GpTimer = &timerTable;
/*----------------------------------------------------------------------------*/
static struct GpTimer *descriptors[] = {0, 0, 0, 0};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct GpTimer *device = object;
  LPC_TMR_TypeDef *reg = device->reg;

  if (reg->IR & IR_MATCH_INTERRUPT(0)) /* Match 0 */
  {
    if (device->callback)
      device->callback(device->callbackArgument);
    reg->IR = IR_MATCH_INTERRUPT(0); /* Clear flag */
  }
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct GpTimer *device)
{
  assert(channel < sizeof(descriptors));

  if (descriptors[channel])
    return E_BUSY;

  descriptors[channel] = device;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void TIMER16B0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void TIMER16B1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void TIMER32B0_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
void TIMER32B1_ISR(void)
{
  if (descriptors[3])
    descriptors[3]->handler(descriptors[3]);
}
/*----------------------------------------------------------------------------*/
static enum result tmrInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct GpTimerConfig * const config = configPtr;
  struct GpTimer *device = object;

  assert(config->frequency);

  /* Try to set peripheral descriptor */
  device->channel = config->channel;
  if (setDescriptor(device->channel, device) != E_OK)
    return E_ERROR;

  /* Set hardware interrupt handler to default handler */
  device->handler = interruptHandler;

  switch (device->channel)
  {
    case 0:
      sysClockEnable(CLK_CT16B0);
      device->reg = LPC_TMR16B0;
      device->irq = TIMER16B0_IRQ;
      break;
    case 1:
      sysClockEnable(CLK_CT16B1);
      device->reg = LPC_TMR16B1;
      device->irq = TIMER16B1_IRQ;
      break;
    case 2:
      sysClockEnable(CLK_CT32B0);
      device->reg = LPC_TMR32B0;
      device->irq = TIMER32B0_IRQ;
      break;
    case 3:
      sysClockEnable(CLK_CT32B1);
      device->reg = LPC_TMR32B1;
      device->irq = TIMER32B1_IRQ;
      break;
  }

  LPC_TMR_TypeDef *reg = device->reg;

  reg->MCR = 0; /* Reset control register */
  reg->PC = reg->TC = 0; /* Reset internal counters */
  reg->IR = IR_MASK; /* Clear pending interrupts */

  /* Configure prescaler */
  reg->PR = (sysCoreClock / DEFAULT_DIV_VALUE) / config->frequency - 1;
  /* Enable timer/counter */
  reg->TCR = TCR_CEN;

  /* Enable interrupt */
  nvicEnable(device->irq);
  /* Set interrupt priority, lowest by default */
  nvicSetPriority(device->irq, DEFAULT_PRIORITY);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void tmrDeinit(void *object)
{
  const enum sysClockDevice tmrClock[] = {
      CLK_CT16B0, CLK_CT16B1, CLK_CT32B0, CLK_CT32B1
  };
  struct GpTimer *device = object;

  /* Disable interrupt */
  nvicDisable(device->irq);
  /* Disable Timer clock */
  sysClockDisable(tmrClock[device->channel]);
  /* Release external clock pin when used*/
  if (gpioGetKey(&device->input))
    gpioDeinit(&device->input);
  /* Reset Timer descriptor */
  setDescriptor(device->channel, 0);
}
/*----------------------------------------------------------------------------*/
static void tmrCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpTimer *device = object;
  LPC_TMR_TypeDef *reg = device->reg;

  device->callback = callback;
  device->callbackArgument = argument;
  /* Enable or disable Match interrupt and counter reset after each interrupt */
  if (callback)
  {
    reg->IR = IR_MATCH_INTERRUPT(0); /* Clear pending interrupt flag */
    reg->MCR |= MCR_INTERRUPT(0);
  }
  else
    reg->MCR &= ~MCR_INTERRUPT(0);
}
/*----------------------------------------------------------------------------*/
static void tmrControl(void *object, bool state)
{
  LPC_TMR_TypeDef *reg = ((struct GpTimer *)object)->reg;

  if (!state)
  {
    reg->TCR |= TCR_CRES;
    while (reg->TC || reg->PC);
  }
  else
    reg->TCR &= ~TCR_CRES;
}
/*----------------------------------------------------------------------------*/
static void tmrSetFrequency(void *object, uint32_t frequency)
{
  ((LPC_TMR_TypeDef *)((struct GpTimer *)object)->reg)->PR =
      (sysCoreClock / DEFAULT_DIV_VALUE) / frequency - 1;
}
/*----------------------------------------------------------------------------*/
static void tmrSetOverflow(void *object, uint32_t overflow)
{
  LPC_TMR_TypeDef *reg = ((struct GpTimer *)object)->reg;

  /* Set match value and enable timer reset when value is greater than zero */
  reg->MR0 = overflow;
  if (overflow)
    reg->MCR |= MCR_RESET(0);
  else
    reg->MCR &= ~MCR_RESET(0);

  /* Synchronously reset prescaler and counter registers */
  reg->TCR |= TCR_CRES;
  while (reg->TC || reg->PC);
  reg->TCR &= ~TCR_CRES;
}
/*----------------------------------------------------------------------------*/
static uint32_t tmrValue(void *object)
{
  return ((LPC_TMR_TypeDef *)((struct GpTimer *)object)->reg)->TC;
}
