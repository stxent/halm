/*
 * i2c_base.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/i2c_base.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct I2CBlockDescriptor
{
  LPC_I2C_Type *reg;
  /* Interrupt number */
  IrqNumber irq;
  /* Clock to register interface and to peripheral */
  enum SysClockBranch clock;
  /* Reset control identifier */
  enum SysBlockReset reset;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t, struct I2CBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_I2C_NO_DEINIT
static void i2cDeinit(void *);
#else
#  define i2cDeinit deletedDestructorTrap
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
        .irq = I2C0_IRQ,
        .clock = CLK_I2C0,
        .reset = RST_I2C0
    }, {
        .reg = LPC_I2C1,
        .irq = I2C1_IRQ,
        .clock = CLK_I2C1,
        .reset = RST_I2C1
    }, {
        .reg = LPC_I2C2,
        .irq = I2C2_IRQ,
        .clock = CLK_I2C2,
        .reset = RST_I2C2
    }, {
        .reg = LPC_I2C3,
        .irq = I2C3_IRQ,
        .clock = CLK_I2C3,
        .reset = RST_I2C3
    }
};
/*----------------------------------------------------------------------------*/
static struct I2CBase *instances[4] = {NULL};
/*----------------------------------------------------------------------------*/
static bool setInstance(uint8_t channel, struct I2CBase *object)
{
  assert(channel < ARRAY_SIZE(instances));

  if (instances[channel] == NULL)
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
void I2C3_ISR(void)
{
  instances[3]->handler(instances[3]);
}
/*----------------------------------------------------------------------------*/
void i2cConfigPins(struct I2CBase *interface)
{
  static const PinNumber sda0 = PIN(PORT_0, 11); /* I2C0_SDA */
  static const PinNumber scl0 = PIN(PORT_0, 10); /* I2C0_SCL */
  struct Pin pin;

  /* Configure I2C serial clock pin */
  pin = pinInit(interface->scl);
  pinInput(pin);

  if (interface->scl == scl0)
  {
    assert(interface->channel == 0);

    pinSetFunction(pin, PIN_ANALOG_I2C);
    pinSetSlewRate(pin, PIN_SLEW_FAST);
  }
  else
  {
    assert(interface->channel != 0);

    const enum PinMuxIndex mux = PINMUX_I2C1_SCL
        + PINMUX_I2C_STRIDE * (interface->channel - 1);
    pinSetMux(pin, mux);
  }

  /* Configure I2C serial data pin */
  pin = pinInit(interface->sda);
  pinInput(pin);

  if (interface->sda == sda0)
  {
    assert(interface->channel == 0);

    pinSetFunction(pin, PIN_ANALOG_I2C);
    pinSetSlewRate(pin, PIN_SLEW_FAST);
  }
  else
  {
    assert(interface->channel != 0);

    const enum PinMuxIndex mux = PINMUX_I2C1_SDA
        + PINMUX_I2C_STRIDE * (interface->channel - 1);
    pinSetMux(pin, mux);
  }
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2CBase *)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CBaseConfig * const config = configBase;
  struct I2CBase * const interface = object;

  if (!setInstance(config->channel, interface))
    return E_BUSY;

  const struct I2CBlockDescriptor * const entry =
      &i2cBlockEntries[config->channel];

  sysClockEnable(entry->clock);
  sysResetPulse(entry->reset);

  interface->channel = config->channel;
  interface->handler = NULL;
  interface->irq = entry->irq;
  interface->reg = entry->reg;

  /* Configure pins */
  interface->scl = config->scl;
  interface->sda = config->sda;
  i2cConfigPins(interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  const struct I2CBase * const interface = object;

  sysClockDisable(i2cBlockEntries[interface->channel].clock);
  instances[interface->channel] = NULL;
}
#endif
