/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/i2c_base.h>
#include <platform/nxp/lpc13xx/clocking.h>
#include <platform/nxp/lpc13xx/system.h>
#include <platform/nxp/lpc13xx/system_defs.h>
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct I2cBase *);
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *, const void *);
static void i2cDeinit(void *);
/*----------------------------------------------------------------------------*/
static struct I2cBase *descriptors[1] = {0};
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass i2cTable = {
    .size = 0, /* Abstract class */
    .init = i2cInit,
    .deinit = i2cDeinit,

    .callback = 0,
    .get = 0,
    .set = 0,
    .read = 0,
    .write = 0
};
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor i2cPins[] = {
    {
        .key = GPIO_TO_PIN(0, 4), /* SCL */
        .channel = 0,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 5), /* SDA */
        .channel = 0,
        .value = 1
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *I2cBase = &i2cTable;
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t channel, struct I2cBase *interface)
{
  assert(channel < sizeof(descriptors));

  if (descriptors[channel])
    return E_BUSY;

  descriptors[channel] = interface;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void I2C_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(struct I2cBase *interface __attribute__((unused)))
{
  return sysCoreClock;
}
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configPtr)
{
  const struct I2cBaseConfig * const config = configPtr;
  struct I2cBase *interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, interface)) != E_OK)
    return res;

  if ((res = i2cSetupPins(interface, configPtr)) != E_OK)
    return res;

  interface->handler = 0;

  /* Set controller specific parameters */
  sysClockEnable(CLK_I2C);
  LPC_SYSCON->PRESETCTRL |= PRESETCTRL_I2C;
  interface->reg = LPC_I2C;
  interface->irq = I2C_IRQ;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  struct I2cBase *interface = object;

  /* Put peripheral in reset and disable clock */
  LPC_SYSCON->PRESETCTRL &= ~PRESETCTRL_I2C;
  sysClockDisable(CLK_I2C);

  /* Reset I2C descriptor */
  setDescriptor(interface->channel, 0);
}
