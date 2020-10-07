/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/gen_1/i2c_base.h>
#include <halm/platform/nxp/lpc11exx/clocking.h>
#include <halm/platform/nxp/lpc11exx/system.h>
#include <halm/platform/nxp/lpc11exx/system_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct I2CBase *);
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
static struct I2CBase *instance = 0;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct I2CBase *object)
{
  if (!instance)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void I2C_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2CBase *interface __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CBaseConfig * const config = configBase;
  struct I2CBase * const interface = object;

  assert(config->channel == 0);

  if (!setInstance(interface))
    return E_BUSY;

  interface->reg = LPC_I2C;
  interface->irq = I2C_IRQ;
  interface->handler = 0;
  interface->channel = config->channel;

  /* Configure pins */
  i2cConfigPins(interface, configBase);

  sysClockEnable(CLK_I2C);
  LPC_SYSCON->PRESETCTRL |= PRESETCTRL_I2C;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_I2C_NO_DEINIT
static void i2cDeinit(void *object __attribute__((unused)))
{
  /* Put the peripheral into the reset state */
  LPC_SYSCON->PRESETCTRL &= ~PRESETCTRL_I2C;
  sysClockDisable(CLK_I2C);

  instance = 0;
}
#endif
