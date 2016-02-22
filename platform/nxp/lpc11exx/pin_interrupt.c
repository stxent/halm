/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <bits.h>
#include <memory.h>
#include <irq.h>
#include <platform/nxp/lpc11exx/pin_defs.h>
#include <platform/nxp/lpc11exx/system_defs.h>
#include <platform/nxp/pin_interrupt.h>
/*----------------------------------------------------------------------------*/
static inline irqNumber calcVector(uint8_t);
static void processInterrupt(uint8_t);
static void resetDescriptor(uint8_t);
static int8_t setDescriptor(struct PinInterrupt *);
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *, const void *);
static void pinInterruptDeinit(void *);
static void pinInterruptCallback(void *, void (*)(void *), void *);
static void pinInterruptSetEnabled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct InterruptClass pinInterruptTable = {
    .size = sizeof(struct PinInterrupt),
    .init = pinInterruptInit,
    .deinit = pinInterruptDeinit,

    .callback = pinInterruptCallback,
    .setEnabled = pinInterruptSetEnabled
};
/*----------------------------------------------------------------------------*/
const struct InterruptClass * const PinInterrupt = &pinInterruptTable;
static struct PinInterrupt *descriptors[8] = {0};
/*----------------------------------------------------------------------------*/
static inline irqNumber calcVector(uint8_t channel)
{
  return PIN_INT0_IRQ + channel;
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  struct PinInterrupt * const interrupt = descriptors[channel];

  LPC_GPIO_INT->IST |= 1 << channel;

  if (!interrupt)
    return;

  if (interrupt->callback)
    interrupt->callback(interrupt->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void resetDescriptor(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(descriptors));

  descriptors[channel] = 0;
}
/*----------------------------------------------------------------------------*/
static int8_t setDescriptor(struct PinInterrupt *interrupt)
{
  /* Find free interrupt */
  for (unsigned int index = 0; index < ARRAY_SIZE(descriptors); ++index)
  {
    if (compareExchangePointer((void **)(descriptors + index), 0, interrupt))
      return (int8_t)index;
  }

  /* All interrupts are busy */
  return -1;
}
/*----------------------------------------------------------------------------*/
void PIN_INT0_ISR(void)
{
  processInterrupt(0);
}
/*----------------------------------------------------------------------------*/
void PIN_INT1_ISR(void)
{
  processInterrupt(1);
}
/*----------------------------------------------------------------------------*/
void PIN_INT2_ISR(void)
{
  processInterrupt(2);
}
/*----------------------------------------------------------------------------*/
void PIN_INT3_ISR(void)
{
  processInterrupt(3);
}
/*----------------------------------------------------------------------------*/
void PIN_INT4_ISR(void)
{
  processInterrupt(4);
}
/*----------------------------------------------------------------------------*/
void PIN_INT5_ISR(void)
{
  processInterrupt(5);
}
/*----------------------------------------------------------------------------*/
void PIN_INT6_ISR(void)
{
  processInterrupt(6);
}
/*----------------------------------------------------------------------------*/
void PIN_INT7_ISR(void)
{
  processInterrupt(7);
}
/*----------------------------------------------------------------------------*/
static enum result pinInterruptInit(void *object, const void *configBase)
{
  const struct PinInterruptConfig * const config = configBase;
  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;

  assert(pinValid(input));

  /* Try to allocate a new channel */
  const int8_t channel = setDescriptor(interrupt);

  if (channel == -1)
    return E_BUSY;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->channel = channel;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  const uint8_t index = interrupt->channel >> 2;
  const uint32_t mask = 1 << interrupt->channel;

  /* Select pin and port */
  LPC_SYSCON->PINTSEL[index] =
      (LPC_SYSCON->PINTSEL[index] & ~PINTSEL_CHANNEL_MASK(channel))
      | PINTSEL_CHANNEL(channel, input.data.port, input.data.offset);
  /* Configure interrupt as edge sensitive*/
  LPC_GPIO_INT->ISEL &= ~mask;
  /* Configure edge sensitivity options */
  if (config->event == PIN_RISING || config->event == PIN_TOGGLE)
    LPC_GPIO_INT->SIENR = mask;
  if (config->event == PIN_FALLING || config->event == PIN_TOGGLE)
    LPC_GPIO_INT->SIENF = mask;

#ifdef CONFIG_PM
  /* Interrupt will wake the controller from low-power modes */
  LPC_SYSCON->STARTERP0 |= STARTERP0_PINT(interrupt->channel);
#endif

  /* Clear pending interrupt flags */
  LPC_GPIO_INT->IST |= mask;

  /* Enable interrupt */
  const irqNumber irq = calcVector(interrupt->channel);

  irqSetPriority(irq, config->priority);
  irqEnable(irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDeinit(void *object)
{
  const struct PinInterrupt * const interrupt = object;
  const irqNumber irq = calcVector(interrupt->channel);
  const uint32_t mask = 1 << interrupt->pin.offset;

  /* Disable channel interrupt in interrupt controller */
  irqDisable(irq);
  /* Disable interrupt sources */
  LPC_GPIO_INT->CIENR = mask;
  LPC_GPIO_INT->CIENF = mask;

  resetDescriptor(interrupt->channel);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptSetEnabled(void *object, bool state)
{
  const struct PinInterrupt * const interrupt = object;
  const irqNumber irq = calcVector(interrupt->channel);

  if (state)
  {
    LPC_GPIO_INT->IST |= 1 << interrupt->pin.offset;
    irqEnable(irq);
  }
  else
    irqDisable(irq);
}
