/*
 * eadc_dma.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/eadc_defs.h>
#include <halm/platform/numicro/eadc_dma.h>
#include <halm/platform/numicro/pdma_circular.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *);
static void dmaHandler(void *);
static bool dmaSetup(struct EadcDma *, const struct EadcDmaConfig *);
static void resetDmaBuffer(struct EadcDma *);
static bool startConversion(struct EadcDma *);
static void stopConversion(struct EadcDma *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const EadcDma = &(const struct InterfaceClass){
    .size = sizeof(struct EadcDma),
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
static void dmaHandler(void *object)
{
  struct EadcDma * const interface = object;

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct EadcDma *interface,
    const struct EadcDmaConfig *config)
{
  static const struct PdmaSettings dmaSettings = {
      .burst = DMA_BURST_1,
      .priority = DMA_PRIORITY_FIXED,
      .width = DMA_WIDTH_HALFWORD,
      .source = {
          .increment = false
      },
      .destination = {
          .increment = true
      }
  };
  const struct PdmaCircularConfig dmaConfig = {
      .number = 1,
      .event = pdmaGetEventAdc(config->channel),
      .channel = config->dma,
      .oneshot = false,
      .silent = true
  };

  interface->dma = init(PdmaCircular, &dmaConfig);

  if (interface->dma)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static void resetDmaBuffer(struct EadcDma *interface)
{
  NM_EADC_Type * const reg = interface->base.reg;

  dmaAppend(interface->dma, interface->buffer, (const void *)&reg->CURDAT,
      interface->count);
}
/*----------------------------------------------------------------------------*/
static bool startConversion(struct EadcDma *interface)
{
  NM_EADC_Type * const reg = interface->base.reg;
  size_t index = 0;

  /* Reset DMA descriptor */
  resetDmaBuffer(interface);

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

  /* Enable PDMA transfers for configured channels */
  reg->PDMACTL = MASK(interface->count);

  if (dmaEnable(interface->dma) != E_OK)
    return false;

  /* Reconfigure peripheral and start the conversion */
  reg->CTL = interface->base.control;

  return true;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct EadcDma *interface)
{
  NM_EADC_Type * const reg = interface->base.reg;

  reg->CTL = 0;
  reg->PDMACTL = 0;
  dmaDisable(interface->dma);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct EadcDmaConfig * const config = configBase;
  assert(config);
  assert(config->pins);
  assert(config->event != ADC_SOFTWARE && config->event < ADC_EVENT_END);

  const struct EadcBaseConfig baseConfig = {
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct EadcDma * const interface = object;
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

  interface->base.control |= CTL_PDMAEN;
  interface->callback = 0;
  interface->sampling = sampling;

  if (dmaSetup(interface, config))
  {
    if (!config->shared)
      res = startConversion(interface) ? E_OK : E_ERROR;
  }
  else
    res = E_ERROR;

  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct EadcDma * const interface = object;

  stopConversion(interface);
  deinit(interface->dma);

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
  struct EadcDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter,
    void *data __attribute__((unused)))
{
  struct EadcDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
  struct EadcDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      stopConversion(interface);
      return E_OK;

    case IF_ENABLE:
      return startConversion(interface) ? E_OK : E_ERROR;

#ifdef CONFIG_PLATFORM_NUMICRO_ADC_SHARED
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
  struct EadcDma * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  memcpy(buffer, interface->buffer, chunk);
  return chunk;
}
