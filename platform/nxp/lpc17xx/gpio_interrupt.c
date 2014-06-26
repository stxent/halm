/*
 * gpio_interrupt.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <bits.h>
#include <irq.h>
#include <platform/nxp/gpio_interrupt.h>
#include <platform/nxp/lpc17xx/gpio_defs.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static void disableInterrupt(union GpioPin);
static void enableInterrupt(union GpioPin, enum gpioIntMode);
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
const struct InterruptClass * const GpioInterrupt = &gpioIntTable;
static struct GpioInterrupt *descriptors[2] = {0};
/*----------------------------------------------------------------------------*/
static void disableInterrupt(union GpioPin pin)
{
  const uint32_t mask = 1 << pin.offset;

  switch (pin.port)
  {
    case 0:
      LPC_GPIO_INT->ENF0 &= ~mask;
      LPC_GPIO_INT->ENR0 &= ~mask;
      break;
    case 2:
      LPC_GPIO_INT->ENF2 &= ~mask;
      LPC_GPIO_INT->ENR2 &= ~mask;
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void enableInterrupt(union GpioPin pin, enum gpioIntMode mode)
{
  const uint32_t mask = 1 << pin.offset;

  switch (pin.port)
  {
    case 0:
      /* Clear pending interrupt flag */
      LPC_GPIO_INT->CLR0 = mask;
      /* Configure edge sensitivity options */
      if (mode != GPIO_RISING)
        LPC_GPIO_INT->ENF0 |= mask;
      if (mode != GPIO_FALLING)
        LPC_GPIO_INT->ENR0 |= mask;
      break;
    case 2:
      LPC_GPIO_INT->CLR2 = mask;
      if (mode != GPIO_RISING)
        LPC_GPIO_INT->ENF2 |= mask;
      if (mode != GPIO_FALLING)
        LPC_GPIO_INT->ENR2 |= mask;
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void processInterrupt(uint8_t channel)
{
  const struct GpioInterrupt *current;
  uint32_t state;

  switch (channel)
  {
    case 0:
      state = LPC_GPIO_INT->STATR0 | LPC_GPIO_INT->STATF0;
      current = descriptors[0];
      break;

    case 2:
      state = LPC_GPIO_INT->STATR2 | LPC_GPIO_INT->STATF2;
      current = descriptors[1];
      break;

    default:
      return;
  }

  while (current)
  {
    if ((state & (1 << current->pin.offset)) && current->callback)
      current->callback(current->callbackArgument);
    current = current->next;
  }

  switch (channel)
  {
    case 0:
      LPC_GPIO_INT->CLR0 = state;
      break;

    case 2:
      LPC_GPIO_INT->CLR2 = state;
      break;
  }
}
/*----------------------------------------------------------------------------*/
static enum result resetDescriptor(union GpioPin pin)
{
  const uint8_t index = !pin.port ? 0 : 1;
  struct GpioInterrupt *current = descriptors[index];

  if (!current)
    return E_ERROR;

  /* Remove the interrupt from chain */
  if (current->pin.key == pin.key)
  {
    descriptors[index] = descriptors[index]->next;
    if (!descriptors[0] && !descriptors[1])
      irqDisable(EINT3_IRQ);

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
  const uint8_t index = !pin.port ? 0 : 1;
  struct GpioInterrupt *current = descriptors[index];

  /* Attach new interrupt to descriptor chain */
  if (!current)
  {
    if (!descriptors[0] && !descriptors[1])
    {
      /* Initial interrupt configuration */
      sysClockControl(CLK_GPIOINT, DEFAULT_DIV);
      irqEnable(EINT3_IRQ);
    }
    descriptors[index] = interrupt;
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
void EINT3_ISR(void)
{
  if (LPC_GPIO_INT->STATUS & STATUS_P0INT)
    processInterrupt(0);

  if (LPC_GPIO_INT->STATUS & STATUS_P2INT)
    processInterrupt(2);
}
/*----------------------------------------------------------------------------*/
static enum result gpioIntInit(void *object, const void *configPtr)
{
  const struct GpioInterruptConfig * const config = configPtr;
  struct GpioInterrupt * const interrupt = object;
  enum result res;

  const struct Gpio input = gpioInit(config->pin);
  /* External interrupt functionality is available only on two ports */
  if (!gpioGetKey(input) || (input.pin.port != 0 && input.pin.port != 2))
    return E_VALUE;

  gpioInput(input);
  interrupt->pin = input.pin;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interrupt->pin, interrupt)) != E_OK)
    return res;

  interrupt->callback = 0;
  interrupt->mode = config->mode;
  interrupt->next = 0;

  enableInterrupt(interrupt->pin, interrupt->mode);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void gpioIntDeinit(void *object)
{
  const union GpioPin pin = ((struct GpioInterrupt *)object)->pin;

  disableInterrupt(pin);
  resetDescriptor(pin);
}
/*----------------------------------------------------------------------------*/
static void gpioIntCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct GpioInterrupt * const interrupt = object;

  interrupt->callbackArgument = argument;
  interrupt->callback = callback;
}
/*----------------------------------------------------------------------------*/
static void gpioIntSetEnabled(void *object, bool state)
{
  struct GpioInterrupt * const interrupt = object;

  if (state)
    enableInterrupt(interrupt->pin, interrupt->mode);
  else
    disableInterrupt(interrupt->pin);
}
