/*
 * fast_gpio_bus.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/fast_gpio_bus.h>
#include <platform/nxp/lpc11xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static enum result busInit(void *, const void *);
static void busDeinit(void *);
static uint32_t busRead(void *);
static void busWrite(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct GpioBusClass gpioBusTable = {
    .size = sizeof(struct FastGpioBus),
    .init = busInit,
    .deinit = busDeinit,

    .read = busRead,
    .write = busWrite
};
/*----------------------------------------------------------------------------*/
const struct GpioBusClass * const FastGpioBus = &gpioBusTable;
/*----------------------------------------------------------------------------*/
static enum result busInit(void *object, const void *configBase)
{
  const struct FastGpioBusConfig * const config = configBase;
  struct FastGpioBus * const bus = object;

  const struct Pin first = pinInit(config->first);
  const struct Pin last = pinInit(config->last);

  assert(pinValid(first));
  assert(pinValid(last));
  assert(first.data.port == last.data.port);
  assert(first.data.offset < last.data.offset);

  for (unsigned int index = first.data.offset; index < last.data.offset;
      ++index)
  {
    const struct Pin pin = pinInit(PIN(first.data.port, index));

    if (config->direction == PIN_OUTPUT)
    {
      pinOutput(pin, (config->initial >> index) & 1);
      pinSetType(pin, config->type);
      pinSetSlewRate(pin, config->rate);
    }
    else
    {
      pinInput(pin);
    }

    pinSetPull(pin, config->pull);
  }

  bus->mask = BIT_FIELD(MASK(last.data.offset - first.data.offset + 1),
      first.data.offset);
  bus->first = first;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void busDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static uint32_t busRead(void *object)
{
  const struct FastGpioBus * const bus = object;
  const LPC_GPIO_Type * const reg = bus->first.reg;

  return reg->MASKED_ACCESS[bus->mask] >> bus->first.data.offset;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct FastGpioBus * const bus = object;
  LPC_GPIO_Type * const reg = bus->first.reg;

  reg->MASKED_ACCESS[bus->mask] = value << bus->first.data.offset;
}
