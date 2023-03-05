/*
 * eadc.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/eadc.h>
#include <halm/platform/numicro/eadc_defs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *);
static void interruptHandler(void *);
static void startConversion(struct Eadc *);
static void stopConversion(struct Eadc *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_EADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Eadc = &(const struct InterfaceClass){
    .size = sizeof(struct Eadc),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = 0
};
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *pins)
{
  size_t index = 0;

  while (pins[index])
    ++index;

  return index;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Eadc * const interface = object;
  NM_EADC_Type * const reg = interface->base.reg;
  uint16_t *buffer = interface->buffer;

  /* Clear pending interrupt flag */
  reg->STATUS2 = STATUS2_ADIF_MASK | STATUS2_ADOVIF_MASK;

  for (size_t index = 0; index < interface->count; ++index)
    *buffer++ = (uint16_t)reg->DAT[index];

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void startConversion(struct Eadc *interface)
{
  NM_EADC_Type * const reg = interface->base.reg;
  size_t index = 0;

  while (index < interface->count)
  {
    reg->SCTL[index] = interface->sampling
        | SCTL0_3_CHSEL(interface->pins[index].channel);
    ++index;
  }

  while (index < ARRAY_SIZE(reg->SCTL))
  {
    reg->SCTL[index] = 0;
    ++index;
  }

  /* Clear pending interrupt flags */
  reg->STATUS2 = STATUS2_ADIF_MASK | STATUS2_ADCMPF_MASK | STATUS2_ADOVIF_MASK;
  /* Enable interrupt 0 for last configured sampling module */
  reg->INTSRC[0] = 1UL << (interface->count - 1);

  irqSetPriority(interface->base.irq.p0, interface->priority);
  irqClearPending(interface->base.irq.p0);
  irqEnable(interface->base.irq.p0);

  /* Reconfigure peripheral and start the conversion */
  reg->CTL = interface->base.control;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct Eadc *interface)
{
  NM_EADC_Type * const reg = interface->base.reg;

  reg->CTL = 0;
  reg->INTSRC[0] = 0;
  irqDisable(interface->base.irq.p0);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct EadcConfig * const config = configBase;
  assert(config);
  assert(config->pins);
  assert(config->event != ADC_SOFTWARE && config->event < ADC_EVENT_END);
  assert(config->sensitivity != PIN_LOW && config->sensitivity != PIN_HIGH);

  const struct EadcBaseConfig baseConfig = {
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct Eadc * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = EadcBase->init(interface, &baseConfig)) != E_OK)
    return res;

  /* Initialize input pins */
  interface->count = calcPinCount(config->pins);
  assert(interface->count > 0 && interface->count <= 32);

  /* Allocate buffer for conversion results */
  interface->buffer = malloc(sizeof(uint16_t) * interface->count);
  if (!interface->buffer)
    return E_MEMORY;
  memset(interface->buffer, 0, sizeof(uint16_t) * interface->count);

  /* Allocate buffer for pin descriptors */
  interface->pins = malloc(sizeof(struct AdcPin) * interface->count);
  if (!interface->pins)
    return E_MEMORY;
  interface->enabled = adcSetupPins(&interface->base, config->pins,
      interface->pins, interface->count);

  /* Calculate trigger delay settings */
  uint16_t offset = config->offset;
  uint8_t divider = 0;

  while (offset > 255)
  {
    offset >>= 1;
    ++divider;
  }
  assert(divider < 4);

  uint32_t sampling = SCTL0_3_TRGDLYDIV(divider) | SCTL0_3_TRGDLYCNT(offset)
      | SCTL0_3_TRGSEL(config->event) | SCTL0_3_EXTSMPT(config->delay);

  if (config->sensitivity != PIN_RISING)
    sampling |= SCTL0_3_EXTFEN;
  if (config->sensitivity != PIN_RISING)
    sampling |= SCTL0_3_EXTREN;

  interface->base.handler = interruptHandler;
  interface->base.control |= CTL_ADCIEN0;
  interface->callback = 0;
  interface->priority = config->priority;
  interface->sampling = sampling;

  if (!config->shared)
    startConversion(interface);

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_EADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct Eadc * const interface = object;

  stopConversion(interface);

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);

  free(interface->pins);
  free(interface->buffer);

  EadcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void adcSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Eadc * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter,
    void *data __attribute__((unused)))
{
  struct Eadc * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
    {
      const struct EadcBase * const instance =
          adcGetInstance(interface->base.channel);

      return (struct EadcBase *)interface == instance ? E_BUSY : E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
  struct Eadc * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      stopConversion(interface);
      return E_OK;

    case IF_ENABLE:
      startConversion(interface);
      return E_OK;

#ifdef CONFIG_PLATFORM_NUMICRO_EADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, 0, object) ? E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, object, 0);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  struct Eadc * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  irqDisable(interface->base.irq.p0);
  memcpy(buffer, interface->buffer, chunk);
  irqEnable(interface->base.irq.p0);

  return chunk;
}
