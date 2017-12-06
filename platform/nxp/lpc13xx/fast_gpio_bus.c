/*
 * fast_gpio_bus.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/fast_gpio_bus.h>
#include <halm/platform/nxp/lpc13xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static enum Result busInit(void *, const void *);
static uint32_t busRead(void *);
static void busWrite(void *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct GpioBusClass gpioBusTable = {
    .size = sizeof(struct FastGpioBus),
    .init = busInit,
    .deinit = 0, /* Default destructor */

    .read = busRead,
    .write = busWrite
};
/*----------------------------------------------------------------------------*/
const struct GpioBusClass * const FastGpioBus = &gpioBusTable;
/*----------------------------------------------------------------------------*/
static enum Result busInit(void *object, const void *configBase)
{
  fastGpioBusConfigPins(object, configBase);
  return E_OK;
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
