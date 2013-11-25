/*
 * adc_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <stdbool.h>
#include <platform/nxp/adc_base.h>
#include <platform/nxp/adc_defs.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/system.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT                   8
/* Pack match channel and pin function in one value */
#define PACK_VALUE(function, channel)   (((channel) << 4) | (function))
/* Unpack function */
#define UNPACK_FUNCTION(value)          ((value) & 0x0F)
/* Unpack match channel */
#define UNPACK_CHANNEL(value)           (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *, const void *);
static void adcDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass adcTable = {
    .size = 0, /* Abstract class */
    .init = adcInit,
    .deinit = adcDeinit,

    .callback = 0,
    .get = 0,
    .set = 0,
    .read = 0,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor adcPins[] = {
    {
        .key = GPIO_TO_PIN(0, 11), /* AD0 */
        .channel = 0,
        .value = PACK_VALUE(2, 0)
    }, {
        .key = GPIO_TO_PIN(1, 0), /* AD1 */
        .channel = 0,
        .value = PACK_VALUE(2, 1)
    }, {
        .key = GPIO_TO_PIN(1, 1), /* AD2 */
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        .key = GPIO_TO_PIN(1, 2), /* AD3 */
        .channel = 0,
        .value = PACK_VALUE(2, 3)
    }, {
        .key = GPIO_TO_PIN(1, 3), /* AD4 */
        .channel = 0,
        .value = PACK_VALUE(2, 4)
    }, {
        .key = GPIO_TO_PIN(1, 4), /* AD5 */
        .channel = 0,
        .value = PACK_VALUE(1, 5)
    }, {
        .key = GPIO_TO_PIN(1, 10), /* AD6 */
        .channel = 0,
        .value = PACK_VALUE(1, 6)
    }, {
        .key = GPIO_TO_PIN(1, 11), /* AD7 */
        .channel = 0,
        .value = PACK_VALUE(1, 7)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *AdcBase = &adcTable;
/*----------------------------------------------------------------------------*/
/* Initialized analog inputs count */
static uint8_t instances = 0;
/*----------------------------------------------------------------------------*/
//uint32_t adcGetClock(struct AdcBase *interface __attribute__((unused)))
//{
//  return sysCoreClock / DEFAULT_DIV_VALUE;
//}
/*----------------------------------------------------------------------------*/
static void blockSetEnabled(bool state)
{
  if (state)
  {
    sysClockEnable(CLK_ADC);
/*    irqSetPriority(ADC_IRQ, priority); TODO ADC: add priority configuration */
    irqEnable(ADC_IRQ);
  }
  else
  {
    irqDisable(ADC_IRQ);
    sysClockDisable(CLK_ADC);
  }
}
/*----------------------------------------------------------------------------*/
static enum result adcInit(void *object, const void *configPtr)
{
  const struct AdcBaseConfig * const config = configPtr;
  const struct GpioDescriptor *pinDescriptor;
  struct AdcBase *interface = object;

  if (!(pinDescriptor = gpioFind(adcPins, config->pin, 0)))
    return E_VALUE;

  interface->channel = UNPACK_CHANNEL(pinDescriptor->value);
  /* Initialize analog input pin */
  struct Gpio pin = gpioInit(config->pin);
  gpioInput(pin);
  /* Enable analog pin mode bit */
  gpioSetFunction(pin, GPIO_ANALOG);
  /* Set analog pin function */
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  /* Clear Power Down bit for the ADC block */
  sysPowerEnable(PWR_ADC);
  /* Enable clock to the ADC */
  sysClockEnable(CLK_ADC);

  /* Set system clock divider */
  LPC_ADC->CR = CR_CLKDIV(sysCoreClock / 45e5); //FIXME Magic numbers

//  if (!instances++)
//    blockSetEnabled(true);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcDeinit(void *object)
{
  struct AdcBase *interface = object;

//  if (!--instances)
//    blockSetEnabled(false);
}
