/*
 * halm/platform/bouffalo/bl602/i2c_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/delay.h>
#include <halm/platform/bouffalo/clocking.h>
#include <halm/platform/bouffalo/i2c_base.h>
#include <halm/platform/bouffalo/i2c_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define RECOVERY_CYCLES 10
/* High and low time in microseconds during bus recover */
#define RECOVERY_TIME   20
/*----------------------------------------------------------------------------*/
static bool setInstance(struct I2CBase *);
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);

#ifndef CONFIG_PLATFORM_BOUFFALO_I2C_NO_DEINIT
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
static struct I2CBase *instance = nullptr;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct I2CBase *object)
{
  if (instance == nullptr)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
[[gnu::interrupt]] void I2C_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
void i2cConfigPins(const struct I2CBaseConfig *config)
{
  const PinNumber pinArray[] = {config->sda, config->scl};

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (pinArray[index])
    {
      const struct Pin pin = pinInit(pinArray[index]);

      pinInput(pin);
      pinSetFunction(pin, PIN_I2C_FUNCTION);
    }
  }
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetClock(const struct I2CBase *)
{
  return clockFrequency(I2CClock);
}
/*----------------------------------------------------------------------------*/
uint32_t i2cGetRate(const struct I2CBase *base)
{
  const BL_I2C_Type * const reg = base->reg;
  const uint32_t phase = PRD_DATA_PRDDPH0_VALUE(reg->I2C_PRD_DATA) + 1;

  return phase ? i2cGetClock(base) / (4 * phase) : 0;
}
/*----------------------------------------------------------------------------*/
void i2cRecoverBus(struct I2CBase *base)
{
  const struct Pin sda = pinInit(base->sda);
  const struct Pin scl = pinInit(base->scl);

  /* Release any clock-stretching slave by toggling SCL */
  pinOutput(scl, true);
  pinOutput(sda, true);

  for (unsigned int number = 0; number < RECOVERY_CYCLES; ++number)
  {
    udelay(RECOVERY_TIME);
    pinReset(scl);
    udelay(RECOVERY_TIME);
    pinSet(scl);
  }

  /* Restore the I2C pin functions */
  pinInput(scl);
  pinSetFunction(scl, PIN_I2C_FUNCTION);
  pinInput(sda);
  pinSetFunction(sda, PIN_I2C_FUNCTION);
}
/*----------------------------------------------------------------------------*/
void i2cSetRate(struct I2CBase *base, uint32_t rate)
{
  const uint32_t clock = i2cGetClock(base);
  assert(rate != 0 && clock != 0 && rate <= clock / 4);

  /* 
   * The phase duration should be shorter than 16 clock cycles
   * for the glitch filter to work correctly.
   */
  const uint32_t phase = clock / (4 * rate) - 1;
  assert(phase < 16);

  BL_I2C_Type * const reg = base->reg;

  reg->I2C_PRD_START = PRD_START_PRDSPH0(phase) | PRD_START_PRDSPH1(phase)
      | PRD_START_PRDSPH2(phase) | PRD_START_PRDSPH3(phase);
  reg->I2C_PRD_STOP = PRD_STOP_PRDPPH0(phase) | PRD_STOP_PRDPPH1(phase)
      | PRD_STOP_PRDPPH2(phase) | PRD_STOP_PRDPPH3(phase);
  reg->I2C_PRD_DATA = PRD_DATA_PRDDPH0(phase) | PRD_DATA_PRDDPH1(phase)
      | PRD_DATA_PRDDPH2(phase) | PRD_DATA_PRDDPH3(phase);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CBaseConfig * const config = configBase;
  struct I2CBase * const interface = object;

  assert(config->channel == 0);
  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  i2cConfigPins(config);

  interface->channel = 0;
  interface->sda = config->sda;
  interface->scl = config->scl;
  interface->handler = nullptr;
  interface->irq = I2C_IRQ;
  interface->reg = BL_I2C;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_I2C_NO_DEINIT
static void i2cDeinit(void *)
{
  instance = nullptr;
}
#endif
