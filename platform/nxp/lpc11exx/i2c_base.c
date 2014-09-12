/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/i2c_base.h>
#include <platform/nxp/lpc11exx/clocking.h>
#include <platform/nxp/lpc11exx/system.h>
#include <platform/nxp/lpc11exx/system_defs.h>
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
  assert(channel < sizeof(descriptors));

  return compareExchangePointer((void **)(descriptors + channel), state,
      interface) ? E_OK : E_BUSY;
}
/*----------------------------------------------------------------------------*/
void I2C_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2cBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock);
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

  interface->handler = 0;

  sysClockEnable(CLK_I2C);
  LPC_SYSCON->PRESETCTRL |= PRESETCTRL_I2C;
  interface->reg = LPC_I2C;
  interface->irq = I2C_IRQ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  struct I2cBase * const interface = object;

  LPC_SYSCON->PRESETCTRL &= ~PRESETCTRL_I2C; /* Put peripheral in reset */
  sysClockDisable(CLK_I2C);
  setDescriptor(interface->channel, interface, 0);
}
