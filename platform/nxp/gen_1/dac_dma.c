/*
 * dac_dma.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/dac_dma.h>
#include <halm/platform/nxp/gen_1/dac_defs.h>
#include <halm/platform/nxp/gpdma_list.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_COUNT 2
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object);
static enum result dmaSetup(struct DacDma *, const struct DacDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *, const void *);
static void dacDeinit(void *);
static enum result dacCallback(void *, void (*)(void *), void *);
static enum result dacGet(void *, enum ifOption, void *);
static enum result dacSet(void *, enum ifOption, const void *);
static size_t dacWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass dacTable = {
    .size = sizeof(struct DacDma),
    .init = dacInit,
    .deinit = dacDeinit,

    .callback = dacCallback,
    .get = dacGet,
    .set = dacSet,
    .read = 0,
    .write = dacWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const DacDma = &dacTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct DacDma * const interface = object;
  LPC_DAC_Type * const reg = interface->base.reg;
  bool event = false;

  if (dmaStatus(interface->dma) != E_BUSY)
  {
    /* Scatter-gather transfer finished */
    reg->CTRL &= ~(CTRL_INT_DMA_REQ | CTRL_CNT_ENA);
    event = true;
  }
  else if ((dmaPending(interface->dma) & 1) == 0)
  {
    event = true;
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct DacDma *interface,
    const struct DacDmaConfig *config)
{
  static const struct GpDmaSettings dmaSettings = {
      .source = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_HALFWORD,
          .increment = true
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_HALFWORD,
          .increment = false
      }
  };
  const struct GpDmaListConfig dmaConfig = {
      .number = BLOCK_COUNT << 1,
      .event = GPDMA_DAC,
      .type = GPDMA_TYPE_M2P,
      .channel = config->dma,
      .silent = false
  };

  interface->dma = init(GpDmaList, &dmaConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaConfigure(interface->dma, &dmaSettings);
  dmaCallback(interface->dma, dmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacInit(void *object, const void *configBase)
{
  const struct DacDmaConfig * const config = configBase;
  assert(config);

  const struct DacBaseConfig baseConfig = {
      .pin = config->pin
  };
  struct DacDma * const interface = object;
  enum result res;

  assert(config->rate);

  /* Call base class constructor */
  if ((res = DacBase->init(object, &baseConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  interface->callback = 0;

  LPC_DAC_Type * const reg = interface->base.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  reg->CTRL = CTRL_DBLBUF_ENA | CTRL_DMA_ENA;
  reg->CNTVAL = (dacGetClock(object) + (config->rate - 1)) / config->rate - 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void dacDeinit(void *object)
{
  struct DacDma * const interface = object;

  deinit(interface->dma);
  DacBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result dacCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DacDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result dacGet(void *object, enum ifOption option, void *data)
{
  struct DacDma * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    case IF_PENDING:
      *(size_t *)data = (dmaPending(interface->dma) + 1) >> 1;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result dacSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t dacWrite(void *object, const void *buffer, size_t length)
{
  struct DacDma * const interface = object;
  const size_t samples = length / sizeof(uint16_t);

  /* At least 2 samples */
  assert(samples >= 2);

  const size_t parts[] = {samples / 2, samples - samples / 2};
  LPC_DAC_Type * const reg = interface->base.reg;
  const uint16_t *source = buffer;

  /* When the transfer is already active it will be continued */
  dmaAppend(interface->dma, (void *)&reg->CR, source, parts[0]);
  source += parts[0];
  dmaAppend(interface->dma, (void *)&reg->CR, source, parts[1]);

  if (dmaStatus(interface->dma) != E_BUSY)
  {
    if (dmaEnable(interface->dma) != E_OK)
    {
      dmaClear(interface->dma);
      return 0;
    }

    /* Enable counter to generate memory access requests */
    reg->CTRL |= CTRL_CNT_ENA;
  }

  return samples * sizeof(uint16_t);
}
