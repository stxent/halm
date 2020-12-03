/*
 * dac.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/dac_defs.h>
#include <halm/platform/lpc/dac.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *, const void *);
static enum Result dacGetParam(void *, int, void *);
static enum Result dacSetParam(void *, int, const void *);
static size_t dacWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacDeinit(void *);
#else
#define dacDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Dac = &(const struct InterfaceClass){
    .size = sizeof(struct Dac),
    .init = dacInit,
    .deinit = dacDeinit,

    .setCallback = 0,
    .getParam = dacGetParam,
    .setParam = dacSetParam,
    .read = 0,
    .write = dacWrite
};
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *object, const void *configBase)
{
  const struct DacConfig * const config = configBase;
  assert(config);

  const struct DacBaseConfig baseConfig = {
      .pin = config->pin
  };
  struct Dac * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = DacBase->init(interface, &baseConfig)) != E_OK)
    return res;

  LPC_DAC_Type * const reg = interface->base.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  /* DMA enable and DAC enable signals are combined on some parts */
  reg->CTRL = CTRL_DMA_ENA;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_DAC_NO_DEINIT
static void dacDeinit(void *object)
{
  DacBase->deinit(object);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result dacGetParam(void *object __attribute__((unused)),
    int parameter __attribute__((unused)), void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result dacSetParam(void *object __attribute__((unused)),
    int parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t dacWrite(void *object, const void *buffer, size_t length)
{
  if (length >= SAMPLE_SIZE)
  {
    struct Dac * const interface = object;
    LPC_DAC_Type * const reg = interface->base.reg;

    reg->CR = (*(const uint16_t *)buffer & CR_OUTPUT_MASK) | CR_BIAS;
    return SAMPLE_SIZE;
  }
  else
    return 0;
}
