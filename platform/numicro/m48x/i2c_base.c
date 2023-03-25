/*
 * i2c_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/i2c_base.h>
#include <halm/platform/numicro/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct I2CBlockDescriptor
{
  NM_I2C_Type *reg;
  /* Clock branch identifier */
  enum SysClockBranch branch;
  /* Reset control identifier */
  enum SysBlockReset reset;
  /* Peripheral interrupt request identifier */
  IrqNumber irq;
};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t);
static bool setInstance(uint8_t, struct I2CBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_I2C_NO_DEINIT
static void i2cDeinit(void *);
#else
#define i2cDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2CBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = i2cInit,
    .deinit = i2cDeinit
};
/*----------------------------------------------------------------------------*/
static const struct I2CBlockDescriptor i2cBlockEntries[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_I2C0
    {
        .reg = NM_I2C0,
        .branch = CLK_I2C0,
        .reset = RST_I2C0,
        .irq = I2C0_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_I2C1
    {
        .reg = NM_I2C1,
        .branch = CLK_I2C1,
        .reset = RST_I2C1,
        .irq = I2C1_IRQ
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_I2C2
    {
        .reg = NM_I2C2,
        .branch = CLK_I2C2,
        .reset = RST_I2C2,
        .irq = I2C2_IRQ
    },
#endif
};
/*----------------------------------------------------------------------------*/
const struct PinEntry i2cPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_I2C0
    /* I2C0_SCL */
    {
        .key = PIN(PORT_A, 5), /* I2C0_SCL */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_B, 5), /* I2C0_SCL */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_B, 9), /* I2C0_SCL */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_C, 1), /* I2C0_SCL */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_C, 12), /* I2C0_SCL */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 7), /* I2C0_SCL */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_E, 13), /* I2C0_SCL */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 3), /* I2C0_SCL */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 0), /* I2C0_SCL */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_H, 2), /* I2C0_SCL */
        .channel = 0,
        .value = 6
    },
    /* I2C0_SDA */
    {
        .key = PIN(PORT_A, 4), /* I2C0_SDA */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_B, 4), /* I2C0_SDA */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_B, 8), /* I2C0_SDA */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_C, 0), /* I2C0_SDA */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_C, 8), /* I2C0_SDA */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_C, 11), /* I2C0_SDA */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_D, 6), /* I2C0_SDA */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_F, 2), /* I2C0_SDA */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_G, 1), /* I2C0_SDA */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_H, 3), /* I2C0_SDA */
        .channel = 0,
        .value = 6
    },
    /* I2C0_SMBAL */
    {
        .key = PIN(PORT_A, 3), /* I2C0_SMBAL */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_C, 3), /* I2C0_SMBAL */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_G, 2), /* I2C0_SMBAL */
        .channel = 0,
        .value = 4
    },
    /* I2C0_SMBSUS */
    {
        .key = PIN(PORT_A, 2), /* I2C0_SMBSUS */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_C, 2), /* I2C0_SMBSUS */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_G, 3), /* I2C0_SMBSUS */
        .channel = 0,
        .value = 4
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_I2C1
    /* I2C1_SCL */
    {
        .key = PIN(PORT_A, 3), /* I2C1_SCL */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_A, 7), /* I2C1_SCL */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_A, 12), /* I2C1_SCL */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_B, 1), /* I2C1_SCL */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_B, 3), /* I2C1_SCL */
        .channel = 1,
        .value = 12
    }, {
        .key = PIN(PORT_B, 11), /* I2C1_SCL */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_C, 5), /* I2C1_SCL */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_D, 5), /* I2C1_SCL */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_E, 1), /* I2C1_SCL */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_F, 0), /* I2C1_SCL */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_G, 2), /* I2C1_SCL */
        .channel = 1,
        .value = 5
    },
    /* I2C1_SDA */
    {
        .key = PIN(PORT_A, 2), /* I2C1_SDA */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_A, 6), /* I2C1_SDA */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_A, 13), /* I2C1_SDA */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_B, 0), /* I2C1_SDA */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_B, 2), /* I2C1_SDA */
        .channel = 1,
        .value = 12
    }, {
        .key = PIN(PORT_B, 10), /* I2C1_SDA */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_C, 4), /* I2C1_SDA */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_D, 4), /* I2C1_SDA */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_E, 0), /* I2C1_SDA */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_F, 1), /* I2C1_SDA */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_G, 3), /* I2C1_SDA */
        .channel = 1,
        .value = 5
    },
    /* I2C1_SMBAL */
    {
        .key = PIN(PORT_B, 9), /* I2C1_SMBAL */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_C, 7), /* I2C1_SMBAL */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_G, 0), /* I2C1_SMBAL */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_H, 8), /* I2C1_SMBAL */
        .channel = 1,
        .value = 8
    },
    /* I2C1_SMBSUS */
    {
        .key = PIN(PORT_B, 8), /* I2C1_SMBSUS */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_C, 6), /* I2C1_SMBSUS */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_G, 1), /* I2C1_SMBSUS */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_H, 9), /* I2C1_SMBSUS */
        .channel = 1,
        .value = 8
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_I2C2
    /* I2C2_SCL */
    {
        .key = PIN(PORT_A, 1), /* I2C2_SCL */
        .channel = 2,
        .value = 9
    }, {
        .key = PIN(PORT_A, 11), /* I2C2_SCL */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_A, 14), /* I2C2_SCL */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_B, 13), /* I2C2_SCL */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_D, 1), /* I2C2_SCL */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_D, 9), /* I2C2_SCL */
        .channel = 2,
        .value = 3
    }, {
        .key = PIN(PORT_H, 8), /* I2C2_SCL */
        .channel = 2,
        .value = 9
    },
    /* I2C2_SDA */
    {
        .key = PIN(PORT_A, 0), /* I2C2_SDA */
        .channel = 2,
        .value = 9
    }, {
        .key = PIN(PORT_A, 10), /* I2C2_SDA */
        .channel = 2,
        .value = 7
    }, {
        .key = PIN(PORT_A, 15), /* I2C2_SDA */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_B, 12), /* I2C2_SDA */
        .channel = 2,
        .value = 8
    }, {
        .key = PIN(PORT_D, 0), /* I2C2_SDA */
        .channel = 2,
        .value = 6
    }, {
        .key = PIN(PORT_D, 8), /* I2C2_SDA */
        .channel = 2,
        .value = 3
    }, {
        .key = PIN(PORT_H, 9), /* I2C2_SDA */
        .channel = 2,
        .value = 9
    },
    /* I2C2_SMBAL */
    {
        .key = PIN(PORT_B, 15), /* I2C2_SMBAL */
        .channel = 2,
        .value = 8
    },
    /* I2C2_SMBSUS */
    {
        .key = PIN(PORT_B, 14), /* I2C2_SMBSUS */
        .channel = 2,
        .value = 8
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct I2CBase *instances[3] = {0};
/*----------------------------------------------------------------------------*/
static uint8_t channelToIndex(uint8_t channel)
{
  uint8_t index = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_I2C0
  if (channel == 0)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_I2C1
  if (channel == 1)
    return index;
  ++index;
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_I2C2
  if (channel == 2)
    return index;
  ++index;
#endif

  return UINT8_MAX;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct I2CBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (!instances[channel])
  {
    instances[channel] = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_I2C0
void I2C0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_I2C1
void I2C1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_I2C2
void I2C2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
#endif
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2CBase *interface)
{
  return clockFrequency(interface->channel == 1 ? Apb1Clock : Apb0Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CBaseConfig * const config = configBase;
  const size_t index = channelToIndex(config->channel);
  struct I2CBase * const interface = object;

  assert(index != UINT8_MAX);
  if (!setInstance(config->channel, interface))
    return E_BUSY;

  const struct I2CBlockDescriptor * const entry = &i2cBlockEntries[index];

  /* Enable clock to peripheral */
  sysClockEnable(entry->branch);
  /* Reset registers to default values */
  sysResetBlock(entry->reset);

  interface->channel = config->channel;
  interface->handler = 0;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  /* Configure pins */
  interface->scl = config->scl;
  interface->sda = config->sda;
  i2cConfigPins(interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  const struct I2CBase * const interface = object;
  const struct I2CBlockDescriptor * const entry =
      &i2cBlockEntries[channelToIndex(interface->channel)];

  sysClockDisable(entry->branch);
  instances[interface->channel] = 0;
}
#endif
