/*
 * gpio_bus.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <halm/gpio_bus.h>
/*----------------------------------------------------------------------------*/
struct SimpleGpioBus
{
  struct GpioBus base;

  /* Pin array */
  struct Pin *pins;
  /* Number of pins in the array */
  uint8_t number;
};
/*----------------------------------------------------------------------------*/
static enum Result busInit(void *, const void *);
static void busDeinit(void *);
static uint32_t busRead(void *);
static void busWrite(void *, uint32_t);
/*----------------------------------------------------------------------------*/
const struct GpioBusClass * const SimpleGpioBus = &(const struct GpioBusClass){
    .size = sizeof(struct SimpleGpioBus),
    .init = busInit,
    .deinit = busDeinit,

    .read = busRead,
    .write = busWrite
};
/*----------------------------------------------------------------------------*/
static enum Result busInit(void *object, const void *configBase)
{
  const struct SimpleGpioBusConfig * const config = configBase;
  struct SimpleGpioBus * const bus = object;
  size_t number = 0;

  /* Find number of pins in the array */
  while (config->pins[number] && ++number < 32);

  assert(number > 0 && number <= 32);

  bus->number = number;
  bus->pins = malloc(sizeof(struct Pin) * bus->number);

  if (!bus->pins)
    return E_MEMORY;

  for (size_t index = 0; index < bus->number; ++index)
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
    {
      pinInput(bus->pins[index]);
    }

    pinSetPull(bus->pins[index], config->pull);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void busDeinit(void *object)
{
  struct SimpleGpioBus * const bus = object;

  free(bus->pins);
}
/*----------------------------------------------------------------------------*/
static uint32_t busRead(void *object)
{
  const struct SimpleGpioBus * const bus = object;
  uint32_t result = 0;

  for (size_t position = 0; position < bus->number; ++position)
    result |= pinRead(bus->pins[position]) << position;

  return result;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  const struct SimpleGpioBus * const bus = object;

  for (size_t position = 0; position < bus->number; ++position)
    pinWrite(bus->pins[position], (value >> position) & 1);
}
