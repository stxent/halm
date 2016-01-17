/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gen_1/i2c_base.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/system.h>
#include <platform/nxp/lpc13xx/system_defs.h>
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
const struct PinEntry i2cPins[] = {
    {
        .key = PIN(0, 4), /* SCL */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(0, 5), /* SDA */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass * const I2cBase = &i2cTable;
static struct I2cBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel,
    const struct I2cBase *state, struct I2cBase *interface)
{
  assert(channel < ARRAY_SIZE(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void I2C_ISR(void)
{
  descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2cBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configBase)
{
  const struct I2cBaseConfig * const config = configBase;
  struct I2cBase * const interface = object;
  enum result res;

  interface->channel = config->channel;
  interface->handler = 0;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interface->channel, 0, interface)) != E_OK)
    return res;

  /* Configure pins */
  i2cConfigPins(interface, configBase);

  sysClockEnable(CLK_I2C);
  LPC_SYSCON->PRESETCTRL |= PRESETCTRL_I2C;
  interface->reg = LPC_I2C;
  interface->irq = I2C_IRQ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  const struct I2cBase * const interface = object;

  LPC_SYSCON->PRESETCTRL &= ~PRESETCTRL_I2C; /* Put peripheral in reset */
  sysClockDisable(CLK_I2C);
  setDescriptor(interface->channel, interface, 0);
}
