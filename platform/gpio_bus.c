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
  struct GpioBus base;

  /* Pin array */
  struct Pin *pins;
  /* Number of pins in array */
  uint8_t number;
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
  unsigned int number = 0;

  /* Find number of pins in the array */
  while (config->pins[number] && ++number < 32);

  if (!number || number >= 32)
    return E_VALUE;

  bus->number = number;
  bus->pins = malloc(sizeof(struct Pin) * bus->number);
  if (!bus->pins)
    return E_MEMORY;

  for (unsigned int index = 0; index < bus->number; ++index)
  {
    bus->pins[index] = pinInit(config->pins[index]);
    if (!pinValid(bus->pins[index]))
    {
      /* Pin does not exist or cannot be used */
      free(bus->pins);
      return E_VALUE;
    }

    if (config->direction == PIN_OUTPUT)
    {
      pinOutput(bus->pins[index], (config->initial >> index) & 1);

      pinSetType(bus->pins[index], config->type);
      pinSetSlewRate(bus->pins[index], config->rate);
    }
    else
      pinInput(bus->pins[index]);

    pinSetPull(bus->pins[index], config->pull);
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

  for (unsigned int position = 0; position < bus->number; ++position)
    result |= pinRead(bus->pins[position]) << position;

  return result;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct CommonGpioBus * const bus = object;

  for (unsigned int position = 0; position < bus->number; ++position)
    pinWrite(bus->pins[position], (value >> position) & 1);
}
