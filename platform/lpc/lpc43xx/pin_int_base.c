/*
 * pin_int_base.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_3/pin_defs.h>
#include <halm/platform/lpc/gen_3/pin_int_base.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
static IrqNumber findIrqNumber(uint8_t);
static int setInstance(struct PinIntBase *);
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
static void pinIntDeinit(void *);
#else
#define pinIntDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const PinIntBase = &(const struct EntityClass){
    .size = sizeof(struct PinIntBase),
    .init = pinIntInit,
    .deinit = pinIntDeinit
};
/*----------------------------------------------------------------------------*/
static struct PinIntBase *instances[8] = {0};
/*----------------------------------------------------------------------------*/
static IrqNumber findIrqNumber(uint8_t channel)
{
  /* In M0APP and M0SUB cores only one interrupt vector is implemented */

  static const IrqNumber IRQ_TABLE[] = {
      PIN_INT0_IRQ,
      PIN_INT1_IRQ,
      PIN_INT2_IRQ,
      PIN_INT3_IRQ,
      PIN_INT4_IRQ,
      PIN_INT5_IRQ,
      PIN_INT6_IRQ,
      PIN_INT7_IRQ
  };

  return IRQ_TABLE[channel];
}
/*----------------------------------------------------------------------------*/
static int setInstance(struct PinIntBase *interrupt)
{
  /* Find free interrupt */
  for (size_t index = 0; index < ARRAY_SIZE(instances); ++index)
  {
    if (!instances[index] && findIrqNumber(index) != -1)
    {
      instances[index] = interrupt;
      return (int)index;
    }
  }

  /* All handlers are busy */
  return -1;
}
/*----------------------------------------------------------------------------*/
void PIN_INT0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT3_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT4_ISR(void)
{
  instances[4]->handler(instances[4]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT5_ISR(void)
{
  instances[5]->handler(instances[5]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT6_ISR(void)
{
  instances[6]->handler(instances[6]);
}
/*----------------------------------------------------------------------------*/
void PIN_INT7_ISR(void)
{
  instances[7]->handler(instances[7]);
}
/*----------------------------------------------------------------------------*/
static enum Result pinIntInit(void *object, const void *configBase)
{
  const struct PinIntBaseConfig * const config = configBase;
  struct PinIntBase * const interrupt = object;
  const int channel = setInstance(interrupt);

  if (channel == -1)
    return E_BUSY;

  interrupt->channel = (uint8_t)channel;
  interrupt->handler = 0;
  interrupt->irq = findIrqNumber(interrupt->channel);

  volatile uint32_t * const reg = LPC_SCU->PINTSEL + (interrupt->channel >> 2);

  *reg = (*reg & ~PINTSEL_CHANNEL_MASK(interrupt->channel))
      | PINTSEL_CHANNEL(interrupt->channel, config->port, config->number);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_PININT_NO_DEINIT
static void pinIntDeinit(void *object)
{
  const struct PinIntBase * const interrupt = object;
  instances[interrupt->channel] = 0;
}
#endif
