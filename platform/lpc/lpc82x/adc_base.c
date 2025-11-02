/*
 * adc_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/adc_base.h>
#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/gen_2/adc_defs.h>
#include <halm/platform/lpc/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define CAL_FREQUENCY 500000
#define MAX_FREQUENCY 30000000
/*----------------------------------------------------------------------------*/
[[gnu::weak]] void adcBaseHandler0A(void);
[[gnu::weak]] void adcBaseHandler0B(void);
[[gnu::weak]] void adcBaseOverrunHandler0(void);
[[gnu::weak]] void adcBaseThresholdHandler0(void);

static uint32_t calcClockDivisor(uint32_t);

static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = adcInit,
    .deinit = adcDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry adcPins[] = {
    {
        .key = PIN(0, 4), /* ADC_11 */
        .channel = 0,
        .value = 11
    }, {
        .key = PIN(0, 6), /* ADC_1 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 7), /* ADC_0 */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(0, 13), /* ADC_10 */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(0, 14), /* ADC_2 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(0, 17), /* ADC_9 */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(0, 18), /* ADC_8 */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(0, 19), /* ADC_7 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(0, 20), /* ADC_6 */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(0, 21), /* ADC_5 */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(0, 22), /* ADC_4 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(0, 23), /* ADC_3 */
        .channel = 0,
        .value = 3
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct AdcBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
void ADC_SEQA_ISR(void)
{
  adcBaseHandler0A();
}
/*----------------------------------------------------------------------------*/
void ADC_SEQB_ISR(void)
{
  adcBaseHandler0B();
}
/*----------------------------------------------------------------------------*/
void ADC_THCMP_ISR(void)
{
  adcBaseThresholdHandler0();
}
/*----------------------------------------------------------------------------*/
void ADC_OVR_ISR(void)
{
  adcBaseOverrunHandler0();
}
/*----------------------------------------------------------------------------*/
void adcBaseHandler0A(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void adcBaseHandler0B(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void adcBaseOverrunHandler0(void)
{
  if (instances[0] != NULL && instances[0]->handler != NULL)
    instances[0]->handler(instances[0]);
  if (instances[1] != NULL && instances[1]->handler != NULL)
    instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
void adcBaseThresholdHandler0(void)
{
  if (instances[0] != NULL && instances[0]->handler != NULL)
    instances[0]->handler(instances[0]);
  if (instances[1] != NULL && instances[1]->handler != NULL)
    instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct AdcBase *, PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(adcPins, key, 0);
  assert(pinEntry != NULL);

  /* Enable fixed pin function */
  const struct Pin pin = pinInit(key);
  pinInput(pin);
  pinSetFunction(pin, PIN_ANALOG);

  return (struct AdcPin){pinEntry->value};
}
/*----------------------------------------------------------------------------*/
void adcEnterCalibrationMode(struct AdcBase *)
{
  const uint32_t divisor = calcClockDivisor(CAL_FREQUENCY);
  assert(divisor && divisor <= CTRL_CLKDIV_MAX + 1);

  LPC_ADC->CTRL = CTRL_CLKDIV(divisor - 1);
}
/*----------------------------------------------------------------------------*/
struct AdcBase *adcGetInstance(enum AdcSequence sequence)
{
  assert(sequence < ARRAY_SIZE(instances));
  return instances[sequence];
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(struct AdcPin)
{
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(enum AdcSequence sequence, struct AdcBase *expected,
    struct AdcBase *interface)
{
  assert(sequence < ARRAY_SIZE(instances));

  if (compareExchangePointer(&instances[sequence], &expected, interface))
  {
    /* All sequence instances should have the same global configuration */
    assert(interface == NULL
        || instances[!(sequence & 1)] == NULL
        || instances[!(sequence & 1)]->control == interface->control);

    if (interface != NULL)
      LPC_ADC->CTRL = interface->control;
    else if (instances[!(sequence & 1)] == NULL)
      LPC_ADC->CTRL = CTRL_CLKDIV(CTRL_CLKDIV_MAX) | CTRL_LPWRMODE;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static uint32_t calcClockDivisor(uint32_t frequency)
{
  const uint32_t fAdc = frequency ? frequency : MAX_FREQUENCY;
  const uint32_t fApb = clockFrequency(MainClock);

  return (fApb + (fAdc - 1)) / fAdc;
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  assert(config->frequency <= MAX_FREQUENCY);
  assert(config->sequence < ADC_SEQ_END);
  assert(!config->accuracy || config->accuracy == 12);

  struct AdcBase * const interface = object;

  if (!config->shared && !adcSetInstance(config->sequence, NULL, interface))
    return E_BUSY;

  if (!sysPowerStatus(PWR_ADC))
  {
    sysPowerEnable(PWR_ADC);
    sysClockEnable(CLK_ADC);
    sysResetPulse(RST_ADC);

    LPC_ADC->CTRL = CTRL_CLKDIV(CTRL_CLKDIV_MAX) | CTRL_LPWRMODE;
  }

  interface->sequence = config->sequence;
  interface->handler = NULL;
  interface->irq.ovr = ADC_OVR_IRQ;
  interface->irq.seq = config->sequence == ADC0_SEQA ?
      ADC_SEQA_IRQ : ADC_SEQB_IRQ;
  interface->irq.thcmp = ADC_THCMP_IRQ;
  interface->reg = LPC_ADC;

  const uint32_t divisor = calcClockDivisor(config->frequency);
  assert(divisor && divisor <= CTRL_CLKDIV_MAX + 1);

  interface->control = CTRL_CLKDIV(divisor - 1);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_ADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct AdcBase * const interface = object;
  adcSetInstance(interface->sequence, interface, NULL);
}
#endif
