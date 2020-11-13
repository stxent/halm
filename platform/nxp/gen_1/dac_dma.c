/*
 * dac_dma.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/dac_dma.h>
#include <halm/platform/nxp/gen_1/dac_defs.h>
#include <halm/platform/nxp/gpdma_list.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_COUNT 2
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object);
static bool dmaSetup(struct DacDma *, const struct DacDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *, const void *);
static void dacSetCallback(void *, void (*)(void *), void *);
static enum Result dacGetParam(void *, int, void *);
static enum Result dacSetParam(void *, int, const void *);
static size_t dacWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_DAC_NO_DEINIT
static void dacDeinit(void *);
#else
#define dacDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const DacDma = &(const struct InterfaceClass){
    .size = sizeof(struct DacDma),
    .init = dacInit,
    .deinit = dacDeinit,

    .setCallback = dacSetCallback,
    .getParam = dacGetParam,
    .setParam = dacSetParam,
    .read = 0,
    .write = dacWrite
};
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct DacDma * const interface = object;
  bool event = false;

  if (dmaStatus(interface->dma) != E_BUSY)
  {
    LPC_DAC_Type * const reg = interface->base.reg;

    /* Scatter-gather transfer finished, disable further requests */
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
static bool dmaSetup(struct DacDma *interface,
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
      .number = BUFFER_COUNT << 1,
      .event = GPDMA_DAC,
      .type = GPDMA_TYPE_M2P,
      .channel = config->dma,
      .silent = false
  };

  interface->dma = init(GpDmaList, &dmaConfig);

  if (interface->dma)
  {
    dmaConfigure(interface->dma, &dmaSettings);
    dmaSetCallback(interface->dma, dmaHandler, interface);
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static enum Result dacInit(void *object, const void *configBase)
{
  const struct DacDmaConfig * const config = configBase;
  assert(config);

  const struct DacBaseConfig baseConfig = {
      .pin = config->pin
  };
  struct DacDma * const interface = object;
  enum Result res;

  assert(config->rate);

  /* Call base class constructor */
  if ((res = DacBase->init(object, &baseConfig)) != E_OK)
    return res;

  if (!dmaSetup(interface, config))
    return E_ERROR;

  interface->callback = 0;

  LPC_DAC_Type * const reg = interface->base.reg;

  reg->CR = (config->value & CR_OUTPUT_MASK) | CR_BIAS;
  reg->CTRL = CTRL_DBLBUF_ENA | CTRL_DMA_ENA;
  reg->CNTVAL = (dacGetClock(object) + (config->rate - 1)) / config->rate - 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_DAC_NO_DEINIT
static void dacDeinit(void *object)
{
  struct DacDma * const interface = object;

  deinit(interface->dma);
  DacBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void dacSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct DacDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result dacGetParam(void *object, int parameter, void *data)
{
  struct DacDma * const interface = object;

  switch ((enum IfParameter)parameter)
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
static enum Result dacSetParam(void *object __attribute__((unused)),
    int parameter __attribute__((unused)),
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
