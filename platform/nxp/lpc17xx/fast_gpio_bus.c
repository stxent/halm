/*
 * fast_gpio_bus.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/fast_gpio_bus.h>
#include <halm/platform/nxp/lpc17xx/pin_defs.h>
/*----------------------------------------------------------------------------*/
static enum Result busInit(void *, const void *);
static uint32_t busRead(void *);
static void busWrite(void *, uint32_t);
/*----------------------------------------------------------------------------*/
const struct GpioBusClass * const FastGpioBus = &(const struct GpioBusClass){
    .size = sizeof(struct FastGpioBus),
    .init = busInit,
    .deinit = 0, /* Default destructor */

    .read = busRead,
    .write = busWrite
};
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

  return (reg->PIN & bus->mask) >> bus->first.data.offset;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct FastGpioBus * const bus = object;
  LPC_GPIO_Type * const reg = bus->first.reg;

  const uint32_t set = (value << bus->first.data.offset) & bus->mask;
  const uint32_t clear = ~set & bus->mask;

  reg->SET = set;
  reg->CLR = clear;
}
