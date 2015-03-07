/*
 * dac.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/dac.h>
#include <platform/nxp/gen_1/dac_defs.h>
/*----------------------------------------------------------------------------*/
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *, const void *);
static void dacDeinit(void *);
static enum result dacCallback(void *, void (*)(void *), void *);
static enum result dacGet(void *, enum ifOption, void *);
static enum result dacSet(void *, enum ifOption, const void *);
static uint32_t dacWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass dacTable = {
    .size = sizeof(struct Dac),
    .init = dacInit,
    .deinit = dacDeinit,

    .callback = dacCallback,
    .get = dacGet,
    .set = dacSet,
    .read = 0,
    .write = dacWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Dac = &dacTable;
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configBase)
{
  const struct DacConfig * const config = configBase;
  const struct DacBaseConfig parentConfig = {
      .pin = config->pin
  };
  struct Dac * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = DacBase->init(object, &parentConfig)) != E_OK)
    return res;

  LPC_DAC_Type * const reg = interface->parent.reg;

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
static enum result dacCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result dacGet(void *object __attribute__((unused)),
    enum ifOption option, void *data)
{
  switch (option)
  {
    case IF_WIDTH:
      *((uint32_t *)data) = DAC_RESOLUTION;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result dacSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static uint32_t dacWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Dac * const interface = object;
  LPC_DAC_Type * const reg = interface->parent.reg;
  const uint32_t samples = length / SAMPLE_SIZE;

  if (!samples)
    return 0;

  reg->CR = (*((const uint16_t *)buffer) & CR_OUTPUT_MASK) | CR_BIAS;

  return SAMPLE_SIZE;
}
