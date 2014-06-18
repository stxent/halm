/*
 * gpio_bus.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <error.h>
#include <gpio_bus.h>
/*----------------------------------------------------------------------------*/
struct CommonGpioBus
{
  struct GpioBus parent;

  /* Pin array */
  struct Gpio *pins;
  /* Number of pins in array */
  uint8_t count;
};
/*----------------------------------------------------------------------------*/
static enum result busInit(void *, const void *);
static void busDeinit(void *);
static uint32_t busRead(void *);
static void busWrite(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct GpioBusClass gpioBusTable = {
    .size = sizeof(struct CommonGpioBus),
    .init = busInit,
    .deinit = busDeinit,

    .read = busRead,
    .write = busWrite
};
/*----------------------------------------------------------------------------*/
const struct GpioBusClass * const GpioBus = &gpioBusTable;
/*----------------------------------------------------------------------------*/
static enum result busInit(void *object, const void *configPtr)
{
  const struct GpioBusConfig * const config = configPtr;
  struct CommonGpioBus * const bus = object;
  uint8_t position = 0;

  while (config->pins[position] && ++position < 32);
  if (!position || position >= 32)
    return E_VALUE;

  bus->count = position;
  bus->pins = malloc(sizeof(struct Gpio) * bus->count);
  if (!bus->pins)
    return E_MEMORY;

  for (position = 0; position < bus->count; ++position)
  {
    bus->pins[position] = gpioInit(config->pins[position]);
    if (!gpioGetKey(bus->pins[position]))
    {
      /* Pin does not exist or cannot be used */
      free(bus->pins);
      return E_VALUE;
    }

    if (config->direction == GPIO_OUTPUT)
    {
      gpioOutput(bus->pins[position], (config->initial >> position) & 1);

      gpioSetType(bus->pins[position], config->type);
      gpioSetSlewRate(bus->pins[position], config->rate);
    }
    else
      gpioInput(bus->pins[position]);

    gpioSetPull(bus->pins[position], config->pull);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void busDeinit(void *object)
{
  struct CommonGpioBus * const bus = object;

  free(bus->pins);
}
/*----------------------------------------------------------------------------*/
static uint32_t busRead(void *object)
{
  struct CommonGpioBus * const bus = object;
  uint32_t result = 0;

  for (uint8_t position = 0; position < bus->count; ++position)
    result |= gpioRead(bus->pins[position]) << position;

  return result;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct CommonGpioBus * const bus = object;

  for (uint8_t position = 0; position < bus->count; ++position)
    gpioWrite(bus->pins[position], (value >> position) & 1);
}
