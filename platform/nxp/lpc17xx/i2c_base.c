/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/i2c_base.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
struct I2cBlockDescriptor
{
  LPC_I2C_Type *reg;
  enum sysPowerDevice power;
  enum sysClockDevice clock;
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, const struct I2cBase *,
    struct I2cBase *);
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *, const void *);
static void i2cDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass i2cTable = {
    .size = 0, /* Abstract class */
    .init = i2cInit,
    .deinit = i2cDeinit
};
/*----------------------------------------------------------------------------*/
static const struct I2cBlockDescriptor i2cBlockEntries[3] = {
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
static struct I2cBase *descriptors[3] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct I2cBase *state, struct I2cBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void I2C0_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
void I2C1_ISR(void)
{
  if (descriptors[1])
    descriptors[1]->handler(descriptors[1]);
}
/*----------------------------------------------------------------------------*/
void I2C2_ISR(void)
{
  if (descriptors[2])
    descriptors[2]->handler(descriptors[2]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2cBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock) / sysClockDivToValue(DEFAULT_DIV);
}
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configPtr)
{
  const struct I2cBaseConfig * const config = configPtr;
  struct I2cBase * const interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  if ((res = i2cSetupPins(interface, configPtr)) != E_OK)
    return res;

  const struct I2cBlockDescriptor entry = i2cBlockEntries[interface->channel];

  sysPowerEnable(entry.power);
  sysClockControl(entry.clock, DEFAULT_DIV);

  interface->handler = 0;
  interface->irq = I2C0_IRQ + interface->channel;
  interface->reg = entry.reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  const struct I2cBase * const interface = object;

  sysPowerDisable(i2cBlockEntries[interface->channel].power);
  setDescriptor(interface->channel, interface, 0);
}
