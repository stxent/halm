/*
 * adc_oneshot.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/adc.h>
#include <halm/platform/lpc/adc_oneshot.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
static void startCalibration(struct AdcOneShot *);
static uint16_t makeChannelConversion(struct AdcOneShot *);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
static enum Result adcGetParam(void *, int, void *);
static enum Result adcSetParam(void *, int, const void *);
static size_t adcRead(void *, void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const AdcOneShot = &(const struct InterfaceClass){
    .size = sizeof(struct AdcOneShot),
    .init = adcInit,
    .deinit = adcDeinit,

    .setCallback = NULL,
    .getParam = adcGetParam,
    .setParam = adcSetParam,
    .read = adcRead,
    .write = NULL
};
/*----------------------------------------------------------------------------*/
static uint16_t makeChannelConversion(struct AdcOneShot *interface)
{
  /* ADC should be locked to avoid simultaneous access */
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  const unsigned int sequenceB = interface->base.sequence & 1;
  LPC_ADC_Type * const reg = interface->base.reg;
  uint32_t value;

  /* Reconfigure peripheral and start the conversion */
  reg->SEQ_CTRL[sequenceB] = interface->control;

  do
  {
    value = reg->DAT[interface->pin.channel];
  }
  while (!(value & DAT_DATAVALID));

  reg->SEQ_CTRL[sequenceB] = 0;

  return DAT_RESULT_VALUE(value);
}
/*----------------------------------------------------------------------------*/
static void startCalibration(struct AdcOneShot *interface)
{
  assert(adcGetInstance(interface->base.sequence) == &interface->base);

  LPC_ADC_Type * const reg = interface->base.reg;

  /* Reconfigure ADC clock */
  adcEnterCalibrationMode(&interface->base);

  /* Start calibration */
  reg->CTRL |= CTRL_LPWRMODE;
  reg->CTRL |= CTRL_CALMODE;
  reg->CTRL &= ~CTRL_LPWRMODE;
  while (reg->CTRL & CTRL_CALMODE);

  /* Restore configuration */
  reg->CTRL = interface->base.control;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcOneShotConfig * const config = configBase;
  assert(config != NULL);

  const struct AdcBaseConfig baseConfig = {
      .frequency = config->frequency,
      .sequence = config->sequence,
      .accuracy = 0,
      .shared = config->shared
  };
  struct AdcOneShot * const interface = object;

  /* Call base class constructor */
  const enum Result res = AdcBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  /* Enable analog function on the input pin */
  interface->pin = adcConfigPin(&interface->base, config->pin);

  /* Calculate Control register value */
  interface->control = SEQ_CTRL_CHANNEL(interface->pin.channel)
      | SEQ_CTRL_START | SEQ_CTRL_SEQ_ENA;
  if (!(config->sequence & 1))
    interface->control |= SEQ_CTRL_LOWPRIO;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcOneShot * const interface = object;

  adcReleasePin(interface->pin);
  AdcBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result adcGetParam(void *object, int parameter, void *)
{
  const struct AdcOneShot * const interface = object;
  const LPC_ADC_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
      if (adcGetInstance(interface->base.sequence) != &interface->base)
        return E_OK;
#endif

      return (reg->SEQ_CTRL[interface->base.sequence & 1] & SEQ_CTRL_SEQ_ENA) ?
          E_BUSY : E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result adcSetParam(void *object, int parameter, const void *)
{
  struct AdcOneShot * const interface = object;

  switch ((enum ADCParameter)parameter)
  {
    case IF_ADC_CALIBRATE:
      startCalibration(interface);
      return E_OK;

    default:
      break;
  }

#ifdef CONFIG_PLATFORM_LPC_ADC_SHARED
  switch ((enum IfParameter)parameter)
  {
    case IF_ACQUIRE:
      return adcSetInstance(interface->base.sequence, NULL, &interface->base) ?
          E_OK : E_BUSY;

    case IF_RELEASE:
      adcSetInstance(interface->base.sequence, &interface->base, NULL);
      return E_OK;

    default:
      break;
  }
#endif

  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t adcRead(void *object, void *buffer,
    [[maybe_unused]] size_t length)
{
  /* Ensure that the buffer has enough space */
  assert(length >= sizeof(uint16_t));

  const uint16_t value = makeChannelConversion(object);
  memcpy(buffer, &value, sizeof(value));
  return sizeof(value);
}
