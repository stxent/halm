/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <irq.h>
#include <platform/nxp/i2c_base.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/power.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV       CLK_DIV1
#define DEFAULT_DIV_VALUE 1
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct I2cBase *);
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *, const void *);
static void i2cDeinit(void *);
/*----------------------------------------------------------------------------*/
static struct I2cBase *descriptors[] = {0, 0, 0};
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
        .key = GPIO_TO_PIN(0, 0), /* SDA1 */
        .channel = 1,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(0, 1), /* SCL1 */
        .channel = 1,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(0, 10), /* SDA2 */
        .channel = 2,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 11), /* SCL2 */
        .channel = 2,
        .value = 2
    }, {
        .key = GPIO_TO_PIN(0, 19), /* SDA1 */
        .channel = 1,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(0, 20), /* SCL1 */
        .channel = 1,
        .value = 3
    }, {
        .key = GPIO_TO_PIN(0, 27), /* SDA0 */
        .channel = 0,
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 28), /* SCL0 */
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
uint32_t i2cGetClock(struct I2cBase *interface __attribute__((unused)))
{
  return sysCoreClock / DEFAULT_DIV_VALUE;
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

  switch (interface->channel)
  {
    case 0:
      sysPowerEnable(PWR_I2C0);
      sysClockControl(CLK_I2C0, DEFAULT_DIV);
      interface->reg = LPC_I2C0;
      interface->irq = I2C0_IRQ;
      break;
    case 1:
      sysPowerEnable(PWR_I2C1);
      sysClockControl(CLK_I2C1, DEFAULT_DIV);
      interface->reg = LPC_I2C1;
      interface->irq = I2C1_IRQ;
      break;
    case 2:
      sysPowerEnable(PWR_I2C2);
      sysClockControl(CLK_I2C2, DEFAULT_DIV);
      interface->reg = LPC_I2C2;
      interface->irq = I2C2_IRQ;
      break;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  const enum sysPowerDevice i2cPower[] = {
      PWR_I2C0, PWR_I2C1, PWR_I2C2
  };
  struct I2cBase *interface = object;

  /* Disable peripheral power */
  sysPowerDisable(i2cPower[interface->channel]);

  /* Release interface pins */
  gpioDeinit(&interface->sdaPin);
  gpioDeinit(&interface->sclPin);

  /* Reset descriptor */
  setDescriptor(interface->channel, 0);
}
