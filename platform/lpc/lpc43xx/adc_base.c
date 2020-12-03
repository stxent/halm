/*
 * adc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/adc_base.h>
#include <halm/platform/lpc/gen_1/adc_defs.h>
#include <halm/platform/lpc/lpc43xx/clocking.h>
#include <halm/platform/lpc/lpc43xx/system.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct AdcBlockDescriptor
{
  LPC_ADC_Type *reg;
  /* Clock to register interface and to peripheral */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};

struct PinDescriptor
{
  uint8_t number;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY                 4500000
/* Pack and unpack conversion channel and pin function */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)         (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)        ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static struct AdcPin configGroupPin(const struct PinGroupEntry *, PinNumber);
static struct AdcPin configRegularPin(const struct PinEntry *, PinNumber);
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = adcInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static const struct AdcBlockDescriptor adcBlockEntries[] = {
    {
        .reg = LPC_ADC0,
        .irq = ADC0_IRQ,
        .clock = CLK_APB3_ADC0,
        .reset = RST_ADC0
    },
    {
        .reg = LPC_ADC1,
        .irq = ADC1_IRQ,
        .clock = CLK_APB3_ADC1,
        .reset = RST_ADC1
    }
};
/*----------------------------------------------------------------------------*/
/* Separate inputs are unavailable on LPC4370 parts */
const struct PinGroupEntry adcPinGroups[] = {
    {
        /* Separate AD0_0 to AD0_7 */
        .begin = PIN(PORT_ADC, 0),
        .end = PIN(PORT_ADC, 7),
        .channel = 0,
        .value = 0
    }, {
        /* Separate AD1_0 to AD1_7 */
        .begin = PIN(PORT_ADC, 0),
        .end = PIN(PORT_ADC, 7),
        .channel = 1,
        .value = 0
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};

const struct PinEntry adcPins[] = {
    {
        .key = PIN(PORT_4, 1), /* ADC0_1 */
        .channel = 0,
        .value = PACK_VALUE(0, 1)
    }, {
        .key = PIN(PORT_4, 3), /* ADC0_0 */
        .channel = 0,
        .value = PACK_VALUE(0, 0)
    }, {
        .key = PIN(PORT_7, 4), /* ADC0_4 */
        .channel = 0,
        .value = PACK_VALUE(0, 4)
    }, {
        .key = PIN(PORT_7, 5), /* ADC0_3 */
        .channel = 0,
        .value = PACK_VALUE(0, 3)
    }, {
        .key = PIN(PORT_7, 7), /* ADC1_6 */
        .channel = 1,
        .value = PACK_VALUE(0, 6)
    }, {
        .key = PIN(PORT_B, 6), /* ADC0_6 */
        .channel = 0,
        .value = PACK_VALUE(4, 6)
    }, {
        .key = PIN(PORT_C, 0), /* ADC1_1 */
        .channel = 1,
        .value = PACK_VALUE(1, 1)
    }, {
        .key = PIN(PORT_C, 3), /* ADC1_0 */
        .channel = 1,
        .value = PACK_VALUE(4, 0)
    }, {
        .key = PIN(PORT_F, 5), /* ADC1_4 */
        .channel = 1,
        .value = PACK_VALUE(4, 4)
    }, {
        .key = PIN(PORT_F, 6), /* ADC1_3 */
        .channel = 1,
        .value = PACK_VALUE(4, 3)
    }, {
        .key = PIN(PORT_F, 7), /* ADC1_7 */
        .channel = 1,
        .value = PACK_VALUE(4, 7)
    }, {
        .key = PIN(PORT_F, 8), /* ADC0_2 */
        .channel = 0,
        .value = PACK_VALUE(4, 2)
    }, {
        .key = PIN(PORT_F, 9), /* ADC1_2 */
        .channel = 1,
        .value = PACK_VALUE(4, 2)
    }, {
        .key = PIN(PORT_F, 10), /* ADC0_5 */
        .channel = 0,
        .value = PACK_VALUE(4, 5)
    }, {
        .key = PIN(PORT_F, 11), /* ADC1_5 */
        .channel = 1,
        .value = PACK_VALUE(4, 5)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct AdcBase *instances[2] = {0};
/*----------------------------------------------------------------------------*/
static struct AdcPin configGroupPin(const struct PinGroupEntry *group,
    PinNumber key)
{
  const struct PinDescriptor begin = {
      .number = PIN_TO_OFFSET(group->begin),
      .port = PIN_TO_PORT(group->begin)
  };
  const struct PinDescriptor current = {
      .number = PIN_TO_OFFSET(key),
      .port = PIN_TO_PORT(key)
  };

  return (struct AdcPin){
      .channel = current.number - begin.number,
      .control = -1
  };
}
/*----------------------------------------------------------------------------*/
static struct AdcPin configRegularPin(const struct PinEntry *entry,
    PinNumber key)
{
  const uint8_t function = UNPACK_FUNCTION(entry->value);
  const uint8_t index = UNPACK_CHANNEL(entry->value);

  /* Fill pin structure and initialize pin as input */
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  /* Enable analog mode */
  pinSetFunction(pin, PIN_ANALOG);
  /* Set analog pin function */
  pinSetFunction(pin, function);

  /* Route analog input */
  LPC_SCU->ENAIO[entry->channel] |= 1 << index;

  return (struct AdcPin){
      .channel = index,
      .control = entry->channel
  };
}
/*----------------------------------------------------------------------------*/
void ADC0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void ADC1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
struct AdcPin adcConfigPin(const struct AdcBase *unit, PinNumber key)
{
  const struct PinGroupEntry * const group = pinGroupFind(adcPinGroups,
      key, unit->channel);

  if (group)
  {
    return configGroupPin(group, key);
  }
  else
  {
    /* Inputs are connected to both peripherals on parts without ADCHS */
    const struct PinEntry *entry = 0;

    for (unsigned int part = 0; !entry && part < 2; ++part)
      entry = pinFind(adcPins, key, part);
    assert(entry);

    return configRegularPin(entry, key);
  }
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(const struct AdcPin adcPin)
{
  if (adcPin.control != -1)
    LPC_SCU->ENAIO[adcPin.control] &= ~(1UL << adcPin.channel);
}
/*----------------------------------------------------------------------------*/
void adcResetInstance(uint8_t channel)
{
  instances[channel] = 0;
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(uint8_t channel, struct AdcBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  void *expected = 0;
  return compareExchangePointer(&instances[channel], &expected, object);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct AdcBaseConfig * const config = configBase;
  struct AdcBase * const interface = object;

  assert(config->channel < ARRAY_SIZE(instances));
  assert(config->frequency <= MAX_FREQUENCY);
  assert(!config->accuracy || (config->accuracy > 2 && config->accuracy < 11));

  const struct AdcBlockDescriptor * const entry =
      &adcBlockEntries[config->channel];

  interface->irq = entry->irq;
  interface->reg = entry->reg;
  interface->handler = 0;
  interface->channel = config->channel;

  if (!sysClockStatus(entry->clock))
  {
    /* Enable clock to register interface and peripheral */
    sysClockEnable(entry->clock);
    /* Reset registers to default values */
    sysResetEnable(entry->reset);
  }

  const uint32_t ticks = config->accuracy ? (10 - config->accuracy) : 0;
  const uint32_t fAdc = config->frequency ? config->frequency : MAX_FREQUENCY;
  const uint32_t fApb = clockFrequency(Apb3Clock);
  const uint32_t divisor = (fApb + (fAdc - 1)) / fAdc;

  interface->control = CR_PDN | CR_CLKDIV(divisor - 1) | CR_CLKS(ticks);
  return E_OK;
}
