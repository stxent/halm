/*
 * i2c_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bdma_oneshot.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/i2c_base.h>
#include <halm/platform/stm32/stm32f1xx/pin_remap.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct I2CBlockDescriptor
{
  STM_I2C_Type *reg;
  /* Interrupt request identifiers */
  struct
  {
    IrqNumber er;
    IrqNumber ev;
  } irq;
  /* Peripheral clock branch */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral channel */
  uint8_t channel;
};
/*----------------------------------------------------------------------------*/
static const struct I2CBlockDescriptor *findDescriptor(uint8_t);
static bool setInstance(uint8_t, struct I2CBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);

#ifndef CONFIG_PLATFORM_STM32_I2C_NO_DEINIT
static void i2cDeinit(void *);
#else
#  define i2cDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2CBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = i2cInit,
    .deinit = i2cDeinit
};
/*----------------------------------------------------------------------------*/
static const struct I2CBlockDescriptor i2cBlockEntries[] = {
#ifdef CONFIG_PLATFORM_STM32_I2C1
    {
        .reg = STM_I2C1,
        .irq = {
            .er = I2C1_ER_IRQ,
            .ev = I2C1_EV_IRQ
        },
        .clock = CLK_I2C1,
        .reset = RST_I2C1,
        .channel = I2C1
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_I2C2
    {
        .reg = STM_I2C2,
        .irq = {
            .er = I2C2_ER_IRQ,
            .ev = I2C2_EV_IRQ
        },
        .clock = CLK_I2C2,
        .reset = RST_I2C2,
        .channel = I2C2
    }
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry i2cPins[] = {
#ifdef CONFIG_PLATFORM_STM32_I2C1
    {
        .key = PIN(PORT_B, 5), /* I2C1_SMBAI */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 6), /* I2C1_SCL */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 7), /* I2C1_SDA */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 8), /* I2C1_SCL */
        .channel = 0,
        .value = PACK_REMAP(REMAP_I2C1, 1)
    }, {
        .key = PIN(PORT_B, 9), /* I2C1_SDA */
        .channel = 0,
        .value = PACK_REMAP(REMAP_I2C1, 1)
    },
#endif
#ifdef CONFIG_PLATFORM_STM32_I2C2
    {
        .key = PIN(PORT_B, 10), /* I2C2_SCL */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_B, 11), /* I2C2_SDA */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_B, 12), /* I2C2_SMBAI */
        .channel = 1,
        .value = 0
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct I2CBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
static const struct I2CBlockDescriptor *findDescriptor(uint8_t channel)
{
  for (size_t index = 0; index < ARRAY_SIZE(i2cBlockEntries); ++index)
  {
    if (i2cBlockEntries[index].channel == channel)
      return &i2cBlockEntries[index];
  }

  return NULL;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct I2CBase *object)
{
  if (instances[channel] == NULL)
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_I2C1
void I2C1_ER_ISR(void)
{
  instances[0]->handler(instances[0]);
}

void I2C1_EV_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_I2C2
void I2C2_ER_ISR(void)
{
  instances[1]->handler(instances[1]);
}

void I2C2_EV_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2CBase *interface __attribute__((unused)))
{
  return clockFrequency(Apb1Clock);
}
/*----------------------------------------------------------------------------*/
void *i2cMakeOneShotDma(uint8_t channel __attribute__((unused)), uint8_t stream,
    enum DmaPriority priority, enum DmaType type)
{
  const struct BdmaOneShotConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(BdmaOneShot, &config);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CBaseConfig * const config = configBase;
  struct I2CBase * const interface = object;

  const struct I2CBlockDescriptor * const entry =
      findDescriptor(config->channel);

  assert(entry != NULL);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  sysClockEnable(entry->clock);
  sysResetEnable(entry->reset);
  sysResetDisable(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq.er = entry->irq.er;
  interface->irq.ev = entry->irq.ev;
  interface->reg = entry->reg;

  /* Configure pins */
  interface->scl = config->scl;
  interface->sda = config->sda;
  i2cConfigPins(interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  const struct I2CBase * const interface = object;
  const struct I2CBlockDescriptor * const entry =
      findDescriptor(interface->channel);

  sysClockDisable(entry->clock);
  instances[interface->channel] = NULL;
}
#endif
