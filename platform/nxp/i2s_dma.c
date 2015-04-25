/*
 * i2s_dma.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gpdma_list.h>
#include <platform/nxp/i2s_defs.h>
#include <platform/nxp/i2s_dma.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_COUNT 2
#define SAMPLE_SIZE sizeof(uint16_t)
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object);
static enum result dmaSetup(struct I2sDma *, const struct I2sDmaConfig *);
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *, const void *);
static void i2sDeinit(void *);
static enum result i2sCallback(void *, void (*)(void *), void *);
static enum result i2sGet(void *, enum ifOption, void *);
static enum result i2sSet(void *, enum ifOption, const void *);
static uint32_t i2sRead(void *, uint8_t *, uint32_t);
static uint32_t i2sWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass i2sTable = {
    .size = sizeof(struct I2sDma),
    .init = i2sInit,
    .deinit = i2sDeinit,

    .callback = i2sCallback,
    .get = i2sGet,
    .set = i2sSet,
    .read = i2sRead,
    .write = i2sWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const I2sDma = &i2sTable;
/*----------------------------------------------------------------------------*/
static void dmaHandler(void *object)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->parent.reg;
  const uint32_t count = dmaCount(interface->dma);
  const enum result res = dmaStatus(interface->dma);

//  /* Scatter-gather transfer finished */
//  if (res != E_BUSY)
//    reg->CTRL &= ~(CTRL_INT_DMA_REQ | CTRL_CNT_ENA);
//
//  if ((res != E_BUSY || !(count & 1)) && interface->callback)
//    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct I2sDma *interface,
    const struct I2sDmaConfig *config)
{
  const struct GpDmaListConfig channelConfig = {
      .event = GPDMA_I2S0_REQ1 + (config->channel << 1),
      .channel = config->channel,
      .source.increment = true,
      .destination.increment = false,
      .type = GPDMA_TYPE_M2P,
      .burst = DMA_BURST_4,
      .width = DMA_WIDTH_WORD,
      .number = 4,
      .size = config->size >> 1,
      .silent = false
  };

  interface->dma = init(GpDmaList, &channelConfig);
  if (!interface->dma)
    return E_ERROR;
  dmaCallback(interface->dma, dmaHandler, interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result updateRate(struct I2sDma *interface)
{
  LPC_I2S_Type * const reg = interface->parent.reg;
  uint32_t bitrate, masterClock;
  struct I2sRateConfig rateConfig;
  enum result res;

  masterClock = interface->sampleRate;
  bitrate = interface->sampleRate * (1 << (interface->width + 3));
  if (interface->stereo)
  {
    bitrate <<= 1;
    masterClock <<= 8;
  }
  else
    masterClock <<= 7;

  res = i2sCalcRate((struct I2sBase *)interface, masterClock, &rateConfig);
  if (res != E_OK)
    return res;

  const uint8_t divisor = masterClock / bitrate;

  reg->RXBITRATE = divisor - 1;
  reg->RXRATE = RATE_X_DIVIDER(rateConfig.x) | RATE_Y_DIVIDER(rateConfig.y);
  reg->TXBITRATE = divisor - 1;
  reg->TXRATE = RATE_X_DIVIDER(rateConfig.x) | RATE_Y_DIVIDER(rateConfig.y);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *object, const void *configBase)
{
  const struct I2sDmaConfig * const config = configBase;
  const struct I2sBaseConfig parentConfig = {
      .rx = {
          .sck = config->rx.sck,
          .ws = config->rx.ws,
          .sda = config->rx.sda,
          .mclk = config->rx.mclk
      },
      .tx = {
          .sck = config->tx.sck,
          .ws = config->tx.ws,
          .sda = config->tx.sda,
          .mclk = config->tx.mclk
      },
      .channel = config->channel
  };
  struct I2sDma * const interface = object;
  enum result res;

  if (/*!config->frequency || FIXME*/!config->size)
    return E_VALUE;

  /* Call base class constructor */
  if ((res = I2sBase->init(object, &parentConfig)) != E_OK)
    return res;

  if ((res = dmaSetup(interface, config)) != E_OK)
    return res;

  interface->callback = 0;
  interface->sampleRate = config->rate;
  interface->size = config->size;
  interface->stereo = config->stereo;
  interface->width = config->width;

  LPC_I2S_Type * const reg = interface->parent.reg;
  uint8_t width;

  switch (interface->width)
  {
    case I2S_WIDTH_8:
      width = WORDWIDTH_8BIT;
      break;

    case I2S_WIDTH_16:
      width = WORDWIDTH_16BIT;
      break;

    case I2S_WIDTH_32:
      width = WORDWIDTH_32BIT;
      break;

    default:
      return E_VALUE;
  }

  reg->DAO = DAO_WORDWIDTH(width)
      | DAO_WS_HALFPERIOD((1 << (interface->width + 3)) - 1);
  reg->DMA1 = DMA_TX_ENABLE | DMA_TX_DEPTH(4);

  reg->RXMODE = 0;
  reg->TXMODE = 0;

  if (config->rx.mclk)
    reg->RXMODE |= RXMODE_RXMCENA;
  if (config->tx.mclk)
    reg->TXMODE |= TXMODE_TXMCENA;

  updateRate(interface);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2sDeinit(void *object)
{
  struct I2sDma * const interface = object;

  deinit(interface->dma);
  I2sBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result i2sCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2sDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2sGet(void *object, enum ifOption option, void *data)
{
  struct I2sDma * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      return dmaStatus(interface->dma);

    case IF_TX_CAPACITY:
      *(uint32_t *)data = BLOCK_COUNT - ((dmaCount(interface->dma) + 1) >> 1);
      return E_OK;

//    case IF_WIDTH:
//      *((uint32_t *)data) = DAC_RESOLUTION;
//      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result i2sSet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static uint32_t i2sRead(void *object, uint8_t *buffer, uint32_t length)
{

}
/*----------------------------------------------------------------------------*/
static uint32_t i2sWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->parent.reg;

  const bool ongoing = dmaStatus(interface->dma) == E_BUSY;

  /* When previous transfer is ongoing it will be continued */
  const enum result res = dmaStart(interface->dma, (void *)&reg->TXFIFO,
      buffer, length / 4);

  if (res != E_OK)
    return 0;

  return length;
}
