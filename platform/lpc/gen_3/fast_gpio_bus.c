/*
 * fast_gpio_bus.c
 * Copyright (C) 2016, 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/fast_gpio_bus.h>
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
  return (LPC_GPIO->PIN[bus->first.port] & bus->mask) >> bus->first.number;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct FastGpioBus * const bus = object;
  const uint32_t set = (value << bus->first.number) & bus->mask;
  const uint32_t clear = ~set & bus->mask;

  LPC_GPIO->SET[bus->first.port] = set;
  LPC_GPIO->CLR[bus->first.port] = clear;
}
