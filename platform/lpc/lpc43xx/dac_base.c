/*
 * dac_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/dac_base.h>
#include <halm/platform/lpc/gen_1/dac_defs.h>
#include <halm/platform/lpc/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void configOutputPin(PinNumber);
static bool setInstance(struct DacBase *);

#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void releaseOutputPin(PinNumber);
#endif
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacDeinit(void *);
#else
#define dacDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const DacBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = dacInit,
    .deinit = dacDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry dacPins[] = {
    {
        .key = PIN(PORT_4, 4),
        .channel = 0,
        .value = 7
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct DacBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static void configOutputPin(PinNumber key)
{
  const struct PinEntry * const pinEntry = pinFind(dacPins, key, 0);
  assert(pinEntry != NULL);

  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, pinEntry->value);

  /* Enable analog pin function */
  LPC_SCU->ENAIO2 |= 0x01;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void releaseOutputPin(PinNumber key __attribute__((unused)))
{
  /* Disable analog pin function */
  LPC_SCU->ENAIO2 &= ~0x01;
}
#endif
/*----------------------------------------------------------------------------*/
static bool setInstance(struct DacBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
uint32_t dacGetClock(const struct DacBase *interface __attribute__((unused)))
{
  return clockFrequency(Apb3Clock);
}
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *object, const void *configBase)
{
  const struct DacBaseConfig * const config = configBase;
  struct DacBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  /* Enable analog function on the output pin */
  configOutputPin(config->pin);
  /* Enable clock to register interface and peripheral */
  sysClockEnable(CLK_APB3_DAC);
  /* Reset registers to default values */
  sysResetEnable(RST_DAC);

  interface->pin = config->pin;
  interface->reg = LPC_DAC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacDeinit(void *object)
{
  const struct DacBase * const interface = object;

  sysClockDisable(CLK_APB3_DAC);
  releaseOutputPin(interface->pin);

  instance = NULL;
}
#endif
