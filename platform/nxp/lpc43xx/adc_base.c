/*
 * adc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <xcore/memory.h>
#include <halm/platform/nxp/gen_1/adc_base.h>
#include <halm/platform/nxp/gen_1/adc_defs.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/system.h>
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
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY                 4500000
/* Pack and unpack conversion channel and pin function */
#define PACK_VALUE(function, channel) (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)         (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)        ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static void configGroupPin(const struct PinGroupEntry *, PinNumber,
    struct AdcPin *);
static void configRegularPin(const struct PinEntry *, PinNumber,
    struct AdcPin *);
static bool setDescriptor(uint8_t, const struct AdcUnitBase *state,
    struct AdcUnitBase *);
/*----------------------------------------------------------------------------*/
static enum Result adcUnitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcUnitDeinit(void *);
#else
#define adcUnitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = 0, /* Abstract class */
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
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
const struct EntityClass * const AdcUnitBase = &adcUnitTable;
static struct AdcUnitBase *descriptors[2] = {0};
/*----------------------------------------------------------------------------*/
static void configGroupPin(const struct PinGroupEntry *group, PinNumber key,
    struct AdcPin *adcPin)
{
  const struct PinData begin = {
      .offset = PIN_TO_OFFSET(group->begin),
      .port = PIN_TO_PORT(group->begin)
  };
  const struct PinData current = {
      .offset = PIN_TO_OFFSET(key),
      .port = PIN_TO_PORT(key)
  };

  adcPin->channel = current.offset - begin.offset;
  adcPin->control = -1;
}
/*----------------------------------------------------------------------------*/
static void configRegularPin(const struct PinEntry *entry, PinNumber key,
    struct AdcPin *adcPin)
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

  adcPin->channel = index;
  adcPin->control = entry->channel;
}
/*----------------------------------------------------------------------------*/
static bool setDescriptor(uint8_t channel, const struct AdcUnitBase *state,
    struct AdcUnitBase *unit)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state, unit);
}
/*----------------------------------------------------------------------------*/
void ADC0_ISR(void)
{
  struct AdcUnitBase * const descriptor = descriptors[0];

  descriptor->handler(descriptor->instance);
}
/*----------------------------------------------------------------------------*/
void ADC1_ISR(void)
{
  struct AdcUnitBase * const descriptor = descriptors[1];

  descriptor->handler(descriptor->instance);
}
/*----------------------------------------------------------------------------*/
void adcConfigPin(const struct AdcUnitBase *unit, PinNumber key,
    struct AdcPin *adcPin)
{
  const struct PinGroupEntry *group;

  if ((group = pinGroupFind(adcPinGroups, key, unit->channel)))
  {
    configGroupPin(group, key, adcPin);
  }
  else
  {
    /* Inputs are connected to both peripherals on parts without ADCHS */
    const struct PinEntry *entry = 0;

    for (unsigned int part = 0; !entry && part < 2; ++part)
      entry = pinFind(adcPins, key, part);
    assert(entry);

    configRegularPin(entry, key, adcPin);
  }
}
/*----------------------------------------------------------------------------*/
void adcReleasePin(const struct AdcPin adcPin)
{
  if (adcPin.control != -1)
    LPC_SCU->ENAIO[adcPin.control] &= ~(1 << adcPin.channel);
}
/*----------------------------------------------------------------------------*/
static enum Result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitBaseConfig * const config = configBase;
  struct AdcUnitBase * const unit = object;

  unit->channel = config->channel;
  unit->handler = 0;
  unit->instance = 0;

  /* Try to set peripheral descriptor */
  if (!setDescriptor(unit->channel, 0, unit))
    return E_BUSY;

  const struct AdcBlockDescriptor * const entry =
      &adcBlockEntries[unit->channel];

  /* Enable clock to register interface and peripheral */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  unit->irq = entry->irq;
  unit->reg = entry->reg;

  /* Configure peripheral registers */

  assert(!config->accuracy || (config->accuracy > 2 && config->accuracy < 11));
  assert(config->frequency <= MAX_FREQUENCY);

  const uint32_t clocks = config->accuracy ? 10 - config->accuracy : 0;
  const uint32_t frequency =
      config->frequency ? config->frequency : MAX_FREQUENCY;
  const uint32_t divisor =
      (clockFrequency(Apb3Clock) + (frequency - 1)) / frequency;
  LPC_ADC_Type * const reg = unit->reg;

  reg->INTEN = 0;
  reg->CR = CR_PDN | CR_CLKDIV(divisor - 1) | CR_CLKS(clocks);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_ADC_NO_DEINIT
static void adcUnitDeinit(void *object)
{
  const struct AdcUnitBase * const unit = object;

  sysClockDisable(adcBlockEntries[unit->channel].clock);
  setDescriptor(unit->channel, unit, 0);
}
#endif
