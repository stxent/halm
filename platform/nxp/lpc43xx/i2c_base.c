/*
 * i2c_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/gen_1/i2c_base.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct I2CBlockDescriptor
{
  LPC_I2C_Type *reg;
  /* Clock to register interface and to peripheral */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct I2CBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_I2C_NO_DEINIT
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
    {
        .reg = LPC_I2C0,
        .clock = CLK_APB1_I2C0,
        .reset = RST_I2C0
    }, {
        .reg = LPC_I2C1,
        .clock = CLK_APB3_I2C1,
        .reset = RST_I2C1
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry i2cPins[] = {
    {
        .key = PIN(PORT_2, 3), /* I2C1_SDA */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_2, 4), /* I2C1_SCL */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_E, 13), /* I2C1_SDA */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_E, 15), /* I2C1_SCL */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_I2C, PIN_I2C0_SCL), /* I2C0_SCL */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_I2C, PIN_I2C0_SDA), /* I2C0_SDA */
        .channel = 0,
        .value = 0
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct I2CBase *instances[2] = {0};
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
void I2C0_ISR(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void I2C1_ISR(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2CBase *interface)
{
  return clockFrequency(interface->channel == 0 ? Apb1Clock : Apb3Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CBaseConfig * const config = configBase;
  struct I2CBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  if (!setInstance(interface->channel, interface))
    return E_BUSY;

  /* Configure pins */
  i2cConfigPins(interface, configBase);

  const struct I2CBlockDescriptor * const entry =
      &i2cBlockEntries[interface->channel];

  /* Enable clock to register interface and peripheral */
  sysClockEnable(entry->clock);
  /* Reset registers to default values */
  sysResetEnable(entry->reset);

  interface->irq = I2C0_IRQ + interface->channel;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  const struct I2CBase * const interface = object;

  /* Main peripheral bus clock is left enabled */
  sysClockDisable(i2cBlockEntries[interface->channel].clock);

  instances[interface->channel] = 0;
}
#endif
