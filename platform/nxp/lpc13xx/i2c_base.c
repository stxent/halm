/*
 * i2c_base.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "platform/nxp/i2c_base.h"
#include "platform/nxp/i2c_defs.h"
#include "platform/nxp/system.h"
#include "platform/nxp/lpc13xx/interrupts.h"
#include "platform/nxp/lpc13xx/power.h"
/*----------------------------------------------------------------------------*/
/* In LPC13xx UART clock divisor is number from 1 to 255, 0 to disable */
#define DEFAULT_DIV       1
#define DEFAULT_DIV_VALUE 1
/*----------------------------------------------------------------------------*/
static const struct GpioDescriptor i2cPins[] = {
    {
        .key = GPIO_TO_PIN(0, 4), /* SCL */
        .value = 1
    }, {
        .key = GPIO_TO_PIN(0, 5), /* SDA */
        .value = 1
    }, {
        /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(uint8_t, struct I2cBase *);
static inline enum result setupPins(struct I2cBase *, const struct I2cConfig *);
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *, const void *);
static void i2cDeinit(void *);
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
const struct InterfaceClass *I2cBase = &i2cTable;
/*----------------------------------------------------------------------------*/
static struct I2cBase *descriptors[] = {0};
/*----------------------------------------------------------------------------*/
void I2C_ISR(void)
{
  if (descriptors[0])
    descriptors[0]->handler(descriptors[0]);
}
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
static inline enum result setupPins(struct I2cBase *interface,
    const struct I2cConfig *config)
{
  const struct GpioDescriptor *pin;

  /* Setup UART RX pin */
  if (!(pin = gpioFind(i2cPins, config->scl, interface->channel)))
    return E_VALUE;
  interface->sclPin = gpioInit(config->scl, GPIO_INPUT);
  gpioSetFunction(&interface->sclPin, pin->value);

  /* Setup UART TX pin */
  if (!(pin = gpioFind(i2cPins, config->sda, interface->channel)))
    return E_VALUE;
  interface->sdaPin = gpioInit(config->sda, GPIO_INPUT);
  gpioSetFunction(&interface->sdaPin, pin->value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configPtr)
{
  const struct I2cConfig * const config = configPtr;
  struct I2cBase *interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  interface->channel = config->channel;
  if ((res = setDescriptor(interface->channel, interface)) != E_OK)
    return res;

  /* Configure interface pins */
  if ((res = setupPins(interface, configPtr)) != E_OK)
    return res;

  /* Reset pointer to interrupt handler function */
  interface->handler = 0;

  switch (config->channel)
  {
    case 0:
      sysClockEnable(CLK_I2C);
      LPC_SYSCON->PRESETCTRL |= 1 << 1; //FIXME Add define
      interface->reg = LPC_I2C;
      interface->irq = I2C_IRQ;
      break;
  }

  LPC_I2C_TypeDef *reg = interface->reg;
  /* TODO Fast mode */
  reg->CONCLR = CONCLR_AAC | CONCLR_SIC | CONCLR_STAC | CONCLR_I2ENC;
  /* Duty cycle registers are 16-bit */
  /* TODO Add check and separate function */
  reg->SCLL = reg->SCLH = (sysCoreClock >> 1) / config->rate;
  reg->CONSET = CONSET_I2EN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  struct I2cBase *interface = object;


}
