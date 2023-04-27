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
    .deinit = NULL, /* Default destructor */

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

  return reg->MASKED_ACCESS[bus->mask] >> bus->first.number;
}
/*----------------------------------------------------------------------------*/
static void busWrite(void *object, uint32_t value)
{
  struct FastGpioBus * const bus = object;
  LPC_GPIO_Type * const reg = bus->first.reg;

  reg->MASKED_ACCESS[bus->mask] = value << bus->first.number;
}
