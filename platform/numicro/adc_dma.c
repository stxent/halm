/*
 * adc_dma.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/adc.h>
#include <halm/platform/numicro/adc_defs.h>
#include <halm/platform/numicro/adc_dma.h>
#include <halm/platform/numicro/pdma_circular.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static size_t calcPinCount(const PinNumber *);
static void dmaHandler(void *);
static bool dmaSetup(struct AdcDma *, const struct AdcDmaConfig *);
static void interruptHandler(void *);
static void resetDmaBuffer(struct AdcDma *);
static void startCalibration(struct AdcDma *);
static bool startConversion(struct AdcDma *);
static void stopCalibration(struct AdcDma *);
static void stopConversion(struct AdcDma *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcDma = &(const struct InterfaceClass){
    .size = sizeof(struct AdcDma),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = adcSetCallback,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = NULL
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
  struct AdcDma * const interface = object;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct AdcDma *interface,
    const struct AdcDmaConfig *config)
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

  if (interface->dma != NULL)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct AdcDma * const interface = object;
  NM_ADC_Type * const reg = interface->base.reg;

  /* Clear pending calibration interrupt flag */
  reg->ADCALSTSR = ADCALSTSR_CALIF;
  /* Disable calibration mode */
  stopCalibration(interface);

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void resetDmaBuffer(struct AdcDma *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  dmaAppend(interface->dma, interface->buffer, (const void *)&reg->ADPDMA,
      interface->count);
}
/*----------------------------------------------------------------------------*/
static void startCalibration(struct AdcDma *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  /* Clear pending calibration interrupt flag */
  reg->ADCALSTSR = ADCALSTSR_CALIF;

  irqSetPriority(interface->base.irq, interface->priority);
  irqClearPending(interface->base.irq);
  irqEnable(interface->base.irq);

  reg->ADCALR = ADCALR_CALEN | ADCALR_CALIE;
  reg->ADCR = interface->base.control | ADCR_ADST;
}
/*----------------------------------------------------------------------------*/
static bool startConversion(struct AdcDma *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  /* Reset DMA descriptor */
  resetDmaBuffer(interface);

  if (dmaEnable(interface->dma) != E_OK)
    return false;

  /* Enable selected channels */
  reg->ADCHER = interface->enabled;
  /* Configure sample time extension */
  reg->ESMPCTL = interface->delay;
  /* Reconfigure peripheral and start the conversion */
  reg->ADCR = interface->base.control;

  return true;
}
/*----------------------------------------------------------------------------*/
static void stopCalibration(struct AdcDma *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  reg->ADCR = 0;
  reg->ADCALR = 0;
  irqDisable(interface->base.irq);
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct AdcDma *interface)
{
  NM_ADC_Type * const reg = interface->base.reg;

  reg->ADCR = 0;
  dmaDisable(interface->dma);
  dmaClear(interface->dma);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL);
  assert(config->event != ADC_SOFTWARE && config->event < ADC_EVENT_END);

  const struct AdcBaseConfig baseConfig = {
      .accuracy = config->accuracy,
      .channel = config->channel,
      .shared = config->shared
  };
  struct AdcDma * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  /* Initialize input pins */
  interface->count = calcPinCount(config->pins);
  assert(interface->count > 0 && interface->count <= 32);

  /* Allocate buffer for conversion results */
  interface->buffer = malloc(sizeof(uint16_t) * interface->count);
  if (interface->buffer == NULL)
    return E_MEMORY;
  memset(interface->buffer, 0, sizeof(uint16_t) * interface->count);

  /* Allocate buffer for pin descriptors */
  interface->pins = malloc(sizeof(struct AdcPin) * interface->count);
  if (interface->pins == NULL)
    return E_MEMORY;
  interface->enabled = adcSetupPins(&interface->base, config->pins,
      interface->pins, interface->count);

  interface->base.handler = interruptHandler;
  interface->base.control |= ADCR_TRGEN | ADCR_PTEN
      | ADCR_ADMD(ADMD_SINGLE_SCAN) | ADCR_TRGS(config->event)
      | ADCR_TRGCOND(adcMakePinCondition(config->sensitivity));

  interface->callback = NULL;
  interface->delay = config->delay;
  interface->priority = config->priority;

  if (!dmaSetup(interface, config))
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

  stopConversion(interface);
  deinit(interface->dma);

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);

  free(interface->pins);
  free(interface->buffer);

  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void adcSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct AdcDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter,
    void *data __attribute__((unused)))
{
  const struct AdcDma * const interface = object;
  const NM_ADC_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      if (adcGetInstance(interface->base.channel) == &interface->base)
      {
        if (!(reg->ADCALR & ADCALR_CALEN))
          return dmaStatus(interface->dma);
        else
          return E_BUSY;
      }
      else
        return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter,
    const void *data __attribute__((unused)))
{
  struct AdcDma * const interface = object;

  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      startCalibration(interface);
      return E_BUSY;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_DISABLE:
      stopConversion(interface);
      return E_OK;

    case IF_ENABLE:
      return startConversion(interface) ? E_OK : E_ERROR;

#ifdef CONFIG_PLATFORM_NUMICRO_ADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.channel, NULL, object) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.channel, object, NULL);
      return E_OK;
#endif

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer, size_t length)
{
  struct AdcDma * const interface = object;
  const size_t chunk = MIN(length, interface->count * sizeof(uint16_t));

  memcpy(buffer, interface->buffer, chunk);
  return chunk;
}
