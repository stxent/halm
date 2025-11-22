/*
 * adc_dma.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/adc.h>
#include <halm/platform/lpc/adc_dma.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <halm/platform/lpc/sdma_circular.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *);
static bool dmaSetup(struct AdcDma *, uint8_t, uint8_t);
static void resetDmaBuffers(struct AdcDma *);
static bool startConversion(struct AdcDma *);
static void stopConversion(struct AdcDma *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static void adcSetCallback(void *, void (*)(void *), void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
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
static void dmaHandler(void *object)
{
  struct AdcDma * const interface = object;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct AdcDma *interface, uint8_t channel,
    uint8_t priority)
{
  static const struct SdmaSettings dmaSettings = {
      .burst = DMA_BURST_1,
      .width = DMA_WIDTH_HALFWORD,
      .source = {
          .stride = SDMA_STRIDE_NONE,
          .wrap = true
      },
      .destination = {
          .stride = SDMA_STRIDE_1,
          .wrap = false
      }
  };
  const struct SdmaCircularConfig dmaConfig = {
      .number = 1,
      .request = SDMA_REQUEST_NONE,
      .trigger = sdmaGetTriggerAdc(interface->base.sequence),
      .channel = channel,
      .priority = priority,
      .oneshot = false,
      .polarity = true,
      .silent = true
  };

  interface->dma = init(SdmaCircular, &dmaConfig);

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
static void resetDmaBuffers(struct AdcDma *interface)
{
  const unsigned int number = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  dmaAppend(interface->dma, interface->buffer,
      (const void *)&reg->SEQ_GDAT[number],
      interface->count * sizeof(uint16_t));
}
/*----------------------------------------------------------------------------*/
static bool startConversion(struct AdcDma *interface)
{
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  const unsigned int number = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Rebuild DMA descriptor chain */
  resetDmaBuffers(interface);

  /* Configure the sequence, but don't enable it */
  reg->SEQ_CTRL[number] = interface->control;

  /* Clear pending DMA request */
  reg->FLAGS = FLAGS_SEQ_INT(number);

  if (dmaEnable(interface->dma) != E_OK)
    return false;

  /* Enable DMA request after each conversion */
  const IrqState state = irqSave();
  reg->INTEN |= INTEN_SEQ_INTEN(number);
  irqRestore(state);

  /* Enable sequence */
  reg->SEQ_CTRL[number] |= SEQ_CTRL_SEQ_ENA;

  return true;
}
/*----------------------------------------------------------------------------*/
static void stopConversion(struct AdcDma *interface)
{
  const unsigned int number = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;

  /* Stop further conversions */
  reg->SEQ_CTRL[number] &= ~SEQ_CTRL_SEQ_ENA;

  /* Disable DMA requests */
  const IrqState state = irqSave();
  reg->INTEN &= ~INTEN_SEQ_INTEN(number);
  irqRestore(state);

  dmaDisable(interface->dma);
  dmaClear(interface->dma);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcDmaConfig * const config = configBase;
  assert(config != NULL);
  assert(config->pins != NULL && *config->pins);
  assert(config->event < ADC_EVENT_END);
  assert(!config->preemption || (config->sequence & 1) == 0);
  assert(config->sensitivity <= INPUT_FALLING);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .sequence = config->sequence,
      .accuracy = config->accuracy,
      .shared = config->shared
  };
  struct AdcDma * const interface = object;
  size_t count = 0;

  for (const PinNumber *pin = config->pins; *pin; ++pin)
    ++count;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  if (!dmaSetup(interface, config->dma, config->priority))
    return E_ERROR;

  interface->buffer =
      malloc((sizeof(uint16_t) + sizeof(struct AdcPin)) * count);
  if (interface->buffer == NULL)
    return E_MEMORY;
  interface->pins = (struct AdcPin *)(interface->buffer + count);

  interface->callback = NULL;
  interface->count = (uint8_t)count;

  interface->control = 0;
  if (config->event == ADC_BURST)
    interface->control |= SEQ_CTRL_BURST;
  else
    interface->control |= SEQ_CTRL_TRIGGER(config->event);
  if (config->preemption)
    interface->control |= SEQ_CTRL_LOWPRIO;
  if (config->sensitivity == INPUT_RISING)
    interface->control |= SEQ_CTRL_TRIGPOL;
  if (config->singlestep)
    interface->control |= SEQ_CTRL_SINGLESTEP;

  /* Initialize input pins */
  const uint32_t mask = adcSetupPins(&interface->base, interface->pins,
      config->pins, count);

  interface->control |= SEQ_CTRL_CHANNELS(mask);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcDma * const interface = object;

#  ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  if (adcGetInstance(interface->base.sequence) == &interface->base)
#  endif
  {
    stopConversion(interface);
  }

  for (size_t index = 0; index < interface->count; ++index)
    adcReleasePin(interface->pins[index]);
  free(interface->buffer);

  deinit(interface->dma);
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
static enum Result adcGetParam(void *object, int parameter, void *)
{
  const struct AdcDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct AdcDma * const interface = object;

  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      adcStartCalibration(&interface->base);
      return E_OK;

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

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.sequence, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.sequence, &interface->base, NULL);
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
