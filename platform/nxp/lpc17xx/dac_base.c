/*
 * dac_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
#include <platform/nxp/dac_base.h>
#include <platform/nxp/dac_defs.h>
#include <platform/nxp/lpc17xx/clocking.h>
#include <platform/nxp/lpc17xx/system.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_DIV CLK_DIV1
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(struct DacBase *);
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *, const void *);
static void dacDeinit(void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass dacTable = {
    .size = sizeof(struct DacBase),
    .init = dacInit,
    .deinit = dacDeinit
};
/*----------------------------------------------------------------------------*/
const struct GpioDescriptor dacPins[] = {
    {
        .key = PIN(0, 26),
        .channel = 0,
        .value = 2
    }, {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
const struct EntityClass *DacBase = &dacTable;
static struct DacBase *descriptor = 0;
/*----------------------------------------------------------------------------*/
static enum result setDescriptor(struct DacBase *interface)
{
  return compareExchangePointer((void **)&descriptor, 0, interface) ? E_OK
      : E_BUSY;
}
/*----------------------------------------------------------------------------*/
enum result setupOutputPin(gpio_t key)
{
  const struct GpioDescriptor *pinDescriptor;
  struct Gpio pin;

  if (!(pinDescriptor = gpioFind(dacPins, key, 0)))
    return E_VALUE;
  gpioInput((pin = gpioInit(key)));
  gpioSetFunction(pin, pinDescriptor->value);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configPtr)
{
  const struct DacBaseConfig * const config = configPtr;
  struct DacBase *interface = object;
  enum result res;

  /* Try to set peripheral descriptor */
  if ((res = setDescriptor(interface)) != E_OK)
    return res;

  if ((res = setupOutputPin(config->pin)) != E_OK)
    return res;

  sysClockControl(CLK_DAC, DEFAULT_DIV);

  interface->reg = LPC_DAC;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object __attribute__((unused)))
{
  setDescriptor(0);
}
