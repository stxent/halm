/*
 * dac.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gen_1/dac_defs.h>
#include <halm/platform/nxp/dac.h>
/*----------------------------------------------------------------------------*/
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *, const void *);
static void dacDeinit(void *);
static enum result dacSetCallback(void *, void (*)(void *), void *);
static enum result dacGetParam(void *, enum IfParameter, void *);
static enum result dacSetParam(void *, enum IfParameter, const void *);
static size_t dacWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass dacTable = {
    .size = sizeof(struct Dac),
    .init = dacInit,
    .deinit = dacDeinit,

    .setCallback = dacSetCallback,
    .getParam = dacGetParam,
    .setParam = dacSetParam,
    .read = 0,
    .write = dacWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Dac = &dacTable;
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configBase)
{
  const struct DacConfig * const config = configBase;
  assert(config);

  const struct DacBaseConfig baseConfig = {
      .pin = config->pin
  };
  struct Dac * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = DacBase->init(object, &baseConfig)) != E_OK)
    return res;

  LPC_DAC_Type * const reg = interface->base.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  /* DMA enable and DAC enable signals are combined on some parts */
  reg->CTRL = CTRL_DMA_ENA;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object)
{
  DacBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum result dacSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result dacGetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result dacSetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t dacWrite(void *object, const void *buffer, size_t length)
{
  struct Dac * const interface = object;
  LPC_DAC_Type * const reg = interface->base.reg;

  if (length < SAMPLE_SIZE)
    return 0;

  reg->CR = (*(const uint16_t *)buffer & CR_OUTPUT_MASK) | CR_BIAS;

  return SAMPLE_SIZE;
}
