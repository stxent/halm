/*
 * pin_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/bits.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/lpc11exx/pin_defs.h>
#include <halm/platform/nxp/lpc11exx/system_defs.h>
#include <halm/platform/nxp/pin_interrupt.h>
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(uint8_t);
static void disableInterrupt(const struct PinInterrupt *);
static void enableInterrupt(const struct PinInterrupt *);
static void processInterrupt(uint8_t);
static void resetDescriptor(uint8_t);
static int setDescriptor(struct PinInterrupt *);
/*----------------------------------------------------------------------------*/
static enum Result pinInterruptInit(void *, const void *);
static void pinInterruptDeinit(void *);
static void pinInterruptEnable(void *);
static void pinInterruptDisable(void *);
static void pinInterruptSetCallback(void *, void (*)(void *), void *);
/*----------------------------------------------------------------------------*/
static const struct InterruptClass pinInterruptTable = {
    .size = sizeof(struct PinInterrupt),
    .init = pinInterruptInit,
    .deinit = pinInterruptDeinit,

    .enable = pinInterruptEnable,
    .disable = pinInterruptDisable,
    .setCallback = pinInterruptSetCallback
};
/*----------------------------------------------------------------------------*/
const struct InterruptClass * const PinInterrupt = &pinInterruptTable;
static struct PinInterrupt *descriptors[8] = {0};
/*----------------------------------------------------------------------------*/
static inline IrqNumber calcVector(uint8_t channel)
{
  return PIN_INT0_IRQ + channel;
}
/*----------------------------------------------------------------------------*/
static void disableInterrupt(const struct PinInterrupt *interrupt)
{
  irqDisable(calcVector(interrupt->channel));
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(const struct PinInterrupt *interrupt)
{
  const IrqNumber irq = calcVector(interrupt->channel);

  LPC_GPIO_INT->IST = 1UL << interrupt->pin.offset;
  irqClearPending(irq);
  irqEnable(irq);
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  struct PinInterrupt * const interrupt = descriptors[channel];

  LPC_GPIO_INT->IST = 1UL << channel;

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
static int setDescriptor(struct PinInterrupt *interrupt)
{
  /* Find free interrupt */
  for (size_t index = 0; index < ARRAY_SIZE(descriptors); ++index)
  {
    if (compareExchangePointer((void **)(descriptors + index), 0, interrupt))
      return (int)index;
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
static enum Result pinInterruptInit(void *object, const void *configBase)
{
  const struct PinInterruptConfig * const config = configBase;
  assert(config);

  const struct Pin input = pinInit(config->pin);
  struct PinInterrupt * const interrupt = object;

  assert(pinValid(input));

  /* Try to allocate a new channel */
  const int channel = setDescriptor(interrupt);

  if (channel == -1)
    return E_BUSY;

  /* Configure the pin */
  pinInput(input);
  pinSetPull(input, config->pull);

  interrupt->callback = 0;
  interrupt->channel = channel;
  interrupt->enabled = false;
  interrupt->event = config->event;
  interrupt->pin = input.data;

  const uint8_t index = interrupt->channel >> 2;
  const uint32_t mask = 1UL << interrupt->channel;

  /* Select pin and port */
  LPC_SYSCON->PINTSEL[index] =
      (LPC_SYSCON->PINTSEL[index] & ~PINTSEL_CHANNEL_MASK(channel))
      | PINTSEL_CHANNEL(channel, input.data.port, input.data.offset);
  /* Configure interrupt as edge sensitive */
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

  /* Configure interrupt priority, interrupt is disabled by default */
  irqSetPriority(calcVector(interrupt->channel), config->priority);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDeinit(void *object)
{
  const struct PinInterrupt * const interrupt = object;
  const uint32_t mask = 1UL << interrupt->pin.offset;

  /* Disable channel interrupt in the interrupt controller */
  disableInterrupt(interrupt);
  /* Disable interrupt sources */
  LPC_GPIO_INT->CIENR = mask;
  LPC_GPIO_INT->CIENF = mask;

  resetDescriptor(interrupt->channel);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptEnable(void *object)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->enabled = true;

  if (interrupt->callback)
    enableInterrupt(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptDisable(void *object)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->enabled = false;
  disableInterrupt(interrupt);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct PinInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;

  if (interrupt->enabled && callback)
    enableInterrupt(interrupt);
  else
    disableInterrupt(interrupt);
}
