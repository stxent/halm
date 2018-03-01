/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gen_1/i2c_base.h>
#include <halm/platform/nxp/lpc17xx/clocking.h>
#include <halm/platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct I2cBlockDescriptor
{
  LPC_I2C_Type *reg;
  enum SysBlockPower power;
  enum SysClockBranch clock;
};
/*----------------------------------------------------------------------------*/
static void resetInstance(uint8_t);
static bool setInstance(uint8_t, struct I2cBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NXP_I2C_NO_DEINIT
static void i2cDeinit(void *);
#else
#define i2cDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
static const struct EntityClass i2cTable = {
    .size = 0, /* Abstract class */
    .init = i2cInit,
    .deinit = i2cDeinit
};
/*----------------------------------------------------------------------------*/
static const struct I2cBlockDescriptor i2cBlockEntries[] = {
    {
        .reg = LPC_I2C0,
        .power = PWR_I2C0,
        .clock = CLK_I2C0
    },
    {
        .reg = LPC_I2C1,
        .power = PWR_I2C1,
        .clock = CLK_I2C1
    },
    {
        .reg = LPC_I2C2,
        .power = PWR_I2C2,
        .clock = CLK_I2C2
    }
};
/*----------------------------------------------------------------------------*/
const struct PinEntry i2cPins[] = {
    {
        .key = PIN(0, 0), /* SDA1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(0, 1), /* SCL1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(0, 10), /* SDA2 */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(0, 11), /* SCL2 */
        .channel = 2,
        .value = 2
    }, {
        .key = PIN(0, 19), /* SDA1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(0, 20), /* SCL1 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(0, 27), /* SDA0 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 28), /* SCL0 */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2cBase = &i2cTable;
static struct I2cBase *instances[3] = {0};
/*----------------------------------------------------------------------------*/
static void resetInstance(uint8_t channel)
{
  instances[channel] = 0;
}
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct I2cBase *object)
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
void I2C2_ISR(void)
{
  instances[2]->handler(instances[2]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2cBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2cBaseConfig * const config = configBase;
  struct I2cBase * const interface = object;

  interface->channel = config->channel;
  interface->handler = 0;

  if (!setInstance(interface->channel, interface))
    return E_BUSY;

  /* Configure pins */
  i2cConfigPins(interface, configBase);

  const struct I2cBlockDescriptor * const entry =
      &i2cBlockEntries[interface->channel];

  sysPowerEnable(entry->power);
  sysClockControl(entry->clock, DEFAULT_DIV);

  interface->irq = I2C0_IRQ + interface->channel;
  interface->reg = entry->reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  const struct I2cBase * const interface = object;

  sysPowerDisable(i2cBlockEntries[interface->channel].power);
  resetInstance(interface->channel);
}
#endif
