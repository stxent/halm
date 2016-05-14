/*
 * fast_gpio_bus.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/fast_gpio_bus.h>
#include <platform/nxp/lpc43xx/pin_defs.h>
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

  fastGpioBusConfigPins(bus, config);

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
  const struct PinData data = bus->first.data;

  return (LPC_GPIO->PIN[data.port] & bus->mask) >> data.offset;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct FastGpioBus * const bus = object;
  const struct PinData data = bus->first.data;

  const uint32_t set = (value << data.offset) & bus->mask;
  const uint32_t clear = ~set & bus->mask;

  LPC_GPIO->SET[data.port] = set;
  LPC_GPIO->CLR[data.port] = clear;
}
