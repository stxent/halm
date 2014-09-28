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
  struct Pin *pins;
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
static enum result busInit(void *object, const void *configBase)
{
  const struct GpioBusConfig * const config = configBase;
  struct CommonGpioBus * const bus = object;
  uint8_t position = 0;

  while (config->pins[position] && ++position < 32);
  if (!position || position >= 32)
    return E_VALUE;

  bus->count = position;
  bus->pins = malloc(sizeof(struct Pin) * bus->count);
  if (!bus->pins)
    return E_MEMORY;

  for (position = 0; position < bus->count; ++position)
  {
    bus->pins[position] = pinInit(config->pins[position]);
    if (!pinValid(bus->pins[position]))
    {
      /* Pin does not exist or cannot be used */
      free(bus->pins);
      return E_VALUE;
    }

    if (config->direction == PIN_OUTPUT)
    {
      pinOutput(bus->pins[position], (config->initial >> position) & 1);

      pinSetType(bus->pins[position], config->type);
      pinSetSlewRate(bus->pins[position], config->rate);
    }
    else
      pinInput(bus->pins[position]);

    pinSetPull(bus->pins[position], config->pull);
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
    result |= pinRead(bus->pins[position]) << position;

  return result;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct CommonGpioBus * const bus = object;

  for (uint8_t position = 0; position < bus->count; ++position)
    pinWrite(bus->pins[position], (value >> position) & 1);
}
