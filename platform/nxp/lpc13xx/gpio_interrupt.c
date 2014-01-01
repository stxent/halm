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
static enum result clearDescriptor(struct Gpio);
static void processInterrupt(uint8_t);
static enum result setDescriptor(struct Gpio, struct GpioInterrupt *);
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
static enum result clearDescriptor(struct Gpio input)
{
  struct GpioInterrupt *current = descriptors[input.pin.port];

  /* Remove the interrupt from chain */
  if (!current)
    return E_ERROR;
  if (current->input.pin.key == input.pin.key)
  {
    descriptors[input.pin.port] = descriptors[input.pin.port]->next;
    if (!descriptors[input.pin.port])
      irqDisable(gpioIntIrq[input.pin.port]);
    return E_OK;
  }
  else
  {
    while (current->next)
    {
      if (current->next->input.pin.key == input.pin.key)
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
static void processInterrupt(uint8_t port)
{
  struct GpioInterrupt *current = descriptors[port];
  LPC_GPIO_Type *reg = calcPort(port);
  uint32_t state = reg->MIS;

  while (current)
  {
    if ((state & (1 << current->input.pin.offset)) && current->callback)
      current->callback(current->callbackArgument);
    current = current->next;
  }
  reg->IC = state;
  /* Synchronizer logic causes a delay of 2 clocks */
}
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(struct Gpio input,
    struct GpioInterrupt *interrupt)
{
  struct GpioInterrupt *current = descriptors[input.pin.port];

  assert(gpioGetKey(input));

  /* Attach new interrupt to descriptor chain */
  if (!current)
  {
    irqEnable(gpioIntIrq[input.pin.port]);
    descriptors[input.pin.port] = interrupt;
  }
  else
  {
    while (current->next)
    {
      if (current->input.pin.key == input.pin.key)
        return E_BUSY;
      current = current->next;
    }
    if (current->input.pin.key != input.pin.key)
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

  /* Pin structure should be initialized before setting up descriptor */
  interrupt->input = gpioInit(config->pin);
  gpioInput(interrupt->input);

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interrupt->input, interrupt)) != E_OK)
    return res;

  interrupt->callback = 0;
  interrupt->next = 0;

  LPC_GPIO_Type *reg = calcPort(interrupt->input.pin.port);
  uint32_t mask = 1 << interrupt->input.pin.offset;

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
  struct Gpio input = ((struct GpioInterrupt *)object)->input;
  LPC_GPIO_Type *reg = calcPort(input.pin.port);
  uint32_t mask = 1 << input.pin.offset;

  reg->IE &= ~mask;
  reg->IBE &= ~mask;
  reg->IEV &= ~mask;
  clearDescriptor(input);
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
  struct Gpio input = ((struct GpioInterrupt *)object)->input;
  LPC_GPIO_Type *reg = calcPort(input.pin.port);
  uint32_t mask = 1 << input.pin.offset;

  if (state)
    reg->IE |= mask;
  else
    reg->IE &= ~mask;
}
