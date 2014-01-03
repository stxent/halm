/*
 * gpio_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <platform/nxp/gpio_interrupt.h>
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t);
static void processInterrupt(uint8_t);
static enum result resetDescriptor(union GpioPin);
static enum result setDescriptor(union GpioPin, struct GpioInterrupt *);
/*----------------------------------------------------------------------------*/
static enum result gpioIntInit(void *, const void *);
static void gpioIntDeinit(void *);
static void gpioIntCallback(void *, void (*)(void *), void *);
static void gpioIntSetEnabled(void *, bool);
/*----------------------------------------------------------------------------*/
static const struct InterruptClass gpioIntTable = {
    .size = sizeof(struct GpioInterrupt),
    .init = gpioIntInit,
    .deinit = gpioIntDeinit,

    .callback = gpioIntCallback,
    .setEnabled = gpioIntSetEnabled
};
/*----------------------------------------------------------------------------*/
static const irq_t gpioIntIrq[] = {
    PIOINT0_IRQ, PIOINT1_IRQ, PIOINT2_IRQ, PIOINT3_IRQ
};
/*----------------------------------------------------------------------------*/
const struct InterruptClass *GpioInterrupt = &gpioIntTable;
static struct GpioInterrupt *descriptors[4] = {0};
/*----------------------------------------------------------------------------*/
static inline LPC_GPIO_Type *calcPort(uint8_t port)
{
  return (LPC_GPIO_Type *)((uint32_t)LPC_GPIO0 +
      ((uint32_t)LPC_GPIO1 - (uint32_t)LPC_GPIO0) * port);
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  struct GpioInterrupt *current = descriptors[channel];
  LPC_GPIO_Type *reg = calcPort(channel);
  uint32_t state = reg->MIS;

  while (current)
  {
    if ((state & (1 << current->pin.offset)) && current->callback)
      current->callback(current->callbackArgument);
    current = current->next;
  }
  reg->IC = state;
  /* Synchronizer logic causes a delay of 2 clocks */
}
/*----------------------------------------------------------------------------*/
static enum result resetDescriptor(union GpioPin pin)
{
  struct GpioInterrupt *current = descriptors[pin.port];

  /* Remove the interrupt from chain */
  if (!current)
    return E_ERROR;
  if (current->pin.key == pin.key)
  {
    descriptors[pin.port] = descriptors[pin.port]->next;
    if (!descriptors[pin.port])
      irqDisable(gpioIntIrq[pin.port]);
    return E_OK;
  }
  else
  {
    while (current->next)
    {
      if (current->next->pin.key == pin.key)
      {
        current->next = current->next->next;
        return E_OK;
      }
      current = current->next;
    }
    return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(union GpioPin pin,
    struct GpioInterrupt *interrupt)
{
  struct GpioInterrupt *current = descriptors[pin.port];

  assert(~pin.key);

  /* Attach new interrupt to descriptor chain */
  if (!current)
  {
    irqEnable(gpioIntIrq[pin.port]);
    descriptors[pin.port] = interrupt;
  }
  else
  {
    while (current->next)
    {
      if (current->pin.key == pin.key)
        return E_BUSY;
      current = current->next;
    }
    if (current->pin.key != pin.key)
      current->next = interrupt;
    else
      return E_BUSY;
  }
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void PIOINT0_ISR(void)
{
  processInterrupt(0);
}
/*----------------------------------------------------------------------------*/
void PIOINT1_ISR(void)
{
  processInterrupt(1);
}
/*----------------------------------------------------------------------------*/
void PIOINT2_ISR(void)
{
  processInterrupt(2);
}
/*----------------------------------------------------------------------------*/
void PIOINT3_ISR(void)
{
  processInterrupt(3);
}
/*----------------------------------------------------------------------------*/
static enum result gpioIntInit(void *object, const void *configPtr)
{
  const struct GpioInterruptConfig * const config = configPtr;
  struct GpioInterrupt *interrupt = object;
  enum result res;

  struct Gpio input = gpioInit(config->pin);
  gpioInput(input);
  interrupt->pin = input.pin;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interrupt->pin, interrupt)) != E_OK)
    return res;

  interrupt->callback = 0;
  interrupt->next = 0;

  LPC_GPIO_Type *reg = calcPort(interrupt->pin.port);
  uint32_t mask = 1 << interrupt->pin.offset;

  /* Configure interrupt as edge sensitive*/
  reg->IS &= ~mask;
  /* Configure edge sensitivity options */
  switch (config->mode)
  {
    case GPIO_RISING:
      reg->IEV |= mask;
      break;
    case GPIO_FALLING:
      reg->IEV &= ~mask;
      break;
    case GPIO_TOGGLE:
      reg->IBE |= mask;
      break;
  }
  /* Clear pending interrupt flag */
  reg->IC = mask;
  /* Disable interrupt masking */
  reg->IE |= mask;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void gpioIntDeinit(void *object)
{
  union GpioPin pin = ((struct GpioInterrupt *)object)->pin;
  LPC_GPIO_Type *reg = calcPort(pin.port);
  uint32_t mask = 1 << pin.offset;

  reg->IE &= ~mask;
  reg->IBE &= ~mask;
  reg->IEV &= ~mask;
  resetDescriptor(pin);
}
/*----------------------------------------------------------------------------*/
static void gpioIntCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpioInterrupt *interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
/*----------------------------------------------------------------------------*/
static void gpioIntSetEnabled(void *object, bool state)
{
  union GpioPin pin = ((struct GpioInterrupt *)object)->pin;
  LPC_GPIO_Type *reg = calcPort(pin.port);
  uint32_t mask = 1 << pin.offset;

  if (state)
    reg->IE |= mask;
  else
    reg->IE &= ~mask;
}
