/*
 * adc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <memory.h>
#include <platform/nxp/adc_base.h>
#include <platform/nxp/adc_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
struct AdcBlockDescriptor
{
  LPC_ADC_Type *reg;
  /* Peripheral interrupt request identifier */
  irq_t irq;
  /* Clock to register interface and to peripheral */
  enum sysClockBranch clock;
  /* Reset control identifier */
  enum sysDeviceReset reset;
};
/*----------------------------------------------------------------------------*/
/* Pack or unpack conversion channel and pin function */
#define PACK_VALUE(function, channel)   (((channel) << 4) | (function))
#define UNPACK_CHANNEL(value)           (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)          ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct AdcUnitBase *state,
    struct AdcUnitBase *);
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *, const void *);
static void adcUnitDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass adcUnitTable = {
    .size = sizeof(struct AdcUnitBase),
    .init = adcUnitInit,
    .deinit = adcUnitDeinit
};
/*----------------------------------------------------------------------------*/
static const struct AdcBlockDescriptor adcBlockEntries[4] = {
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
        .value = PACK_VALUE(7, 1)
    }, {
        .key = PIN(PORT_4, 3), /* ADC0_0 */
        .channel = 0,
        .value = PACK_VALUE(7, 0)
    }, {
        .key = PIN(PORT_7, 4), /* ADC0_4 */
        .channel = 0,
        .value = PACK_VALUE(7, 4)
    }, {
        .key = PIN(PORT_7, 5), /* ADC0_3 */
        .channel = 0,
        .value = PACK_VALUE(7, 3)
    }, {
        .key = PIN(PORT_7, 7), /* ADC1_6 */
        .channel = 1,
        .value = PACK_VALUE(7, 6)
    }, {
        .key = PIN(PORT_B, 6), /* ADC0_6 */
        .channel = 0,
        .value = PACK_VALUE(7, 6)
    }, {
        .key = PIN(PORT_C, 0), /* ADC1_1 */
        .channel = 1,
        .value = PACK_VALUE(7, 1)
    }, {
        .key = PIN(PORT_C, 3), /* ADC1_0 */
        .channel = 1,
        .value = PACK_VALUE(7, 0)
    }, {
        .key = PIN(PORT_F, 5), /* ADC1_4 */
        .channel = 1,
        .value = PACK_VALUE(7, 4)
    }, {
        .key = PIN(PORT_F, 6), /* ADC1_3 */
        .channel = 1,
        .value = PACK_VALUE(7, 3)
    }, {
        .key = PIN(PORT_F, 7), /* ADC1_7 */
        .channel = 1,
        .value = PACK_VALUE(7, 7)
    }, {
        .key = PIN(PORT_F, 8), /* ADC0_2 */
        .channel = 0,
        .value = PACK_VALUE(7, 2)
    }, {
        .key = PIN(PORT_F, 9), /* ADC1_2 */
        .channel = 1,
        .value = PACK_VALUE(7, 2)
    }, {
        .key = PIN(PORT_F, 10), /* ADC0_5 */
        .channel = 0,
        .value = PACK_VALUE(7, 5)
    }, {
        .key = PIN(PORT_F, 11), /* ADC1_5 */
        .channel = 1,
        .value = PACK_VALUE(7, 5)
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const AdcUnitBase = &adcUnitTable;
static struct AdcUnitBase *descriptors[2] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct AdcUnitBase *state, struct AdcUnitBase *unit)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      unit) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void ADC0_ISR()
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void ADC1_ISR()
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
int8_t adcSetupPin(uint8_t channel, pin_t key)
{
  const struct PinEntry *entry;
  const struct PinGroupEntry *group;

  group = pinGroupFind(adcPinGroups, key, channel);
  if (group)
  {
    union PinData begin, current;

    begin.key = ~group->begin;
    current.key = ~key;

    return (int8_t)(current.offset - begin.offset);
  }

  /* Inputs are connected to both peripherals on parts without ADCHS */
  for (uint8_t part = 0; part < 2; ++part)
  {
    if (!(entry = pinFind(adcPins, key, part)))
      continue;

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
    *(&LPC_SCU->ENAIO0 + part) |= 1 << index;

    return (int8_t)index;
  }

  return -1;
}
/*----------------------------------------------------------------------------*/
static enum result adcUnitInit(void *object, const void *configBase)
{
  const struct AdcUnitBaseConfig * const config = configBase;
  struct AdcUnitBase * const unit = object;
  enum result res;

  /* Try to set peripheral descriptor */
  unit->channel = config->channel;
  if ((res = setDescriptor(unit->channel, 0, unit)) != E_OK)
    return res;

  const struct AdcBlockDescriptor *entry = &adcBlockEntries[unit->channel];

  /* Enable clock to register interface and peripheral */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  unit->handler = 0;
  unit->irq = entry->irq;
  unit->reg = entry->reg;

  LPC_ADC_Type * const reg = unit->reg;

  /* TODO Change prescaler on power mode changes */
  /* Enable converter and set system clock divider */
  reg->CR = CR_PDN | CR_CLKDIV(clockFrequency(Apb3Clock) / 4500000);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void adcUnitDeinit(void *object)
{
  const struct AdcUnitBase * const unit = object;

  sysClockDisable(adcBlockEntries[unit->channel].clock);
  setDescriptor(unit->channel, unit, 0);
}
