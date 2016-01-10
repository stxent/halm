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
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct I2sDma *, const struct I2sDmaConfig *,
    bool, bool);
static void interruptHandler(void *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static enum result updateRate(struct I2sDma *, bool);
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
static enum result dmaSetup(struct I2sDma *interface,
    const struct I2sDmaConfig *config, bool rx, bool tx)
{
  const uint32_t sizeInBytes = config->size * (1 << config->width)
      * (config->mono ? 1 : 2);
  const struct GpDmaListConfig channelConfigs[] = {
      {
        .event = GPDMA_I2S0_REQ1 + (config->channel << 1),
        .channel = config->rx.dma,
        .source.increment = false,
        .destination.increment = true,
        .type = GPDMA_TYPE_P2M,
        .burst = DMA_BURST_4,
        .width = DMA_WIDTH_WORD,
        .number = BLOCK_COUNT << 1,
        .size = sizeInBytes >> 3, /* Half of the size in 32-bit words */
        .silent = false
      },
      {
        .event = GPDMA_I2S0_REQ2 + (config->channel << 1),
        .channel = config->tx.dma,
        .source.increment = true,
        .destination.increment = false,
        .type = GPDMA_TYPE_M2P,
        .burst = DMA_BURST_4,
        .width = DMA_WIDTH_WORD,
        .number = BLOCK_COUNT << 1,
        .size = sizeInBytes >> 3, /* Half of the size in 32-bit words */
        .silent = false
      }
  };

  if (rx)
  {
    interface->rxDma = init(GpDmaList, &channelConfigs[0]);
    if (!interface->rxDma)
      return E_ERROR;
    dmaCallback(interface->rxDma, rxDmaHandler, interface);
  }
  else
    interface->rxDma = 0;

  if (tx)
  {
    interface->txDma = init(GpDmaList, &channelConfigs[1]);
    if (!interface->txDma)
      return E_ERROR;
    dmaCallback(interface->txDma, txDmaHandler, interface);
  }
  else
    interface->txDma = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;

  if ((reg->STATE & STATE_IRQ) && !(reg->DMA2 & DMA_TX_ENABLE))
  {
    reg->DAO |= DAO_STOP;
    reg->IRQ &= ~IRQ_TX_ENABLE;
  }
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;
  bool event = false;

  if (reg->DMA1 & DMA_RX_ENABLE)
  {
    const uint32_t count = dmaCount(interface->rxDma);
    const enum result res = dmaStatus(interface->rxDma);

    if (res != E_BUSY)
    {
      reg->DAI |= DAI_STOP;
      reg->DMA1 &= ~DMA_RX_ENABLE;
    }
    if (res != E_BUSY || !(count & 1))
      event = true;
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;
  bool event = false;

  if (reg->DMA2 & DMA_TX_ENABLE)
  {
    const uint32_t count = dmaCount(interface->txDma);
    const enum result res = dmaStatus(interface->txDma);

    if (res != E_BUSY)
    {
      /* Workaround to transmit last sample correctly */
      reg->TXFIFO = 0;

      reg->DMA2 &= ~DMA_TX_ENABLE;
    }
    if (res != E_BUSY || !(count & 1))
      event = true;
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result updateRate(struct I2sDma *interface, bool slave)
{
  LPC_I2S_Type * const reg = interface->base.reg;
  uint32_t bitrate, masterClock;
  struct I2sRateConfig rateConfig;
  uint8_t divisor;
  enum result res;

  if (slave)
  {
    rateConfig.x = rateConfig.y = 1;
    divisor = 1;
  }
  else
  {
    masterClock = interface->sampleRate;
    bitrate = interface->sampleRate * (1 << (interface->width + 3));

    if (interface->mono)
    {
      masterClock <<= 7;
    }
    else
    {
      bitrate <<= 1;
      masterClock <<= 8;
    }
    divisor = masterClock / bitrate;

    res = i2sCalcRate((struct I2sBase *)interface, masterClock, &rateConfig);
    if (res != E_OK)
      return res;
  }

  const uint32_t rate = RATE_X_DIVIDER(rateConfig.x)
      | RATE_Y_DIVIDER(rateConfig.y);

  if (interface->rx)
  {
    reg->RXBITRATE = divisor - 1;
    reg->RXRATE = rate;
  }
  if (interface->tx)
  {
    reg->TXBITRATE = divisor - 1;
    reg->TXRATE = rate;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *object, const void *configBase)
{
  const struct I2sDmaConfig * const config = configBase;
  const struct I2sBaseConfig baseConfig = {
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

  if (!config->rate || !config->size)
    return E_VALUE;
  if (!config->rx.sda && !config->tx.sda)
    return E_ERROR;

  /* Call base class constructor */
  if ((res = I2sBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  interface->callback = 0;
  interface->sampleRate = config->rate;
  interface->size = config->size;
  interface->mono = config->mono;
  interface->width = config->width;

  interface->rx = config->rx.sda != 0;
  interface->tx = config->tx.sda != 0;

  if ((res = dmaSetup(interface, config, interface->rx, interface->tx)) != E_OK)
    return res;

  LPC_I2S_Type * const reg = interface->base.reg;
  const uint8_t halfPeriod = (1 << (interface->width + 3)) - 1;
  uint8_t width;

  switch (interface->width)
  {
    case I2S_WIDTH_16:
      width = WORDWIDTH_16BIT;
      break;

    case I2S_WIDTH_32:
      width = WORDWIDTH_32BIT;
      break;

    default:
      width = WORDWIDTH_8BIT;
      break;
  }

  uint32_t dai = DAI_WORDWIDTH(width) | DAI_WS_HALFPERIOD(halfPeriod)
      | DAI_RESET;
  uint32_t dao = DAO_WORDWIDTH(width) | DAO_WS_HALFPERIOD(halfPeriod)
      | DAO_RESET;

  reg->IRQ = 0;
  reg->RXMODE = reg->TXMODE = 0;
  reg->RXRATE = reg->TXRATE = 0;
  reg->RXBITRATE = reg->TXBITRATE = 0;

  if (interface->rx)
  {
    dai |= DAI_STOP;
    reg->DMA1 = DMA_RX_DEPTH(4);

    if (config->slave)
      dai |= DAI_WS_SEL;
    if (interface->mono)
      dai |= DAI_MONO;

    if (config->rx.mclk)
      reg->RXMODE |= RXMODE_RXMCENA;

    if (!config->rx.ws && !config->rx.sck)
      reg->RXMODE |= RXMODE_RX4PIN;
    else if (!config->rx.ws ^ !config->rx.sck)
      return E_VALUE;
  }
  else
  {
    /* Set default values */
    dai |= DAI_WS_SEL;
    reg->DMA1 = 0;
  }

  if (interface->tx)
  {
    dao |= DAO_STOP;
    reg->DMA2 = DMA_TX_DEPTH(3);

    if (config->slave)
      dao |= DAO_WS_SEL;
    if (interface->mono)
      dao |= DAO_MONO;

    if (config->tx.mclk)
      reg->TXMODE |= TXMODE_TXMCENA;

    if (!config->tx.ws && !config->tx.sck)
      reg->TXMODE |= TXMODE_TX4PIN;
    else if (!config->tx.ws ^ !config->tx.sck)
      return E_VALUE;

    reg->IRQ |= IRQ_TX_DEPTH(0);
  }
  else
  {
    /* Set default values */
    dao |= DAO_WS_SEL | DAO_MUTE;
    reg->DMA2 = 0;
  }

  reg->DAO = dao;
  reg->DAI = dai;

  /* Object fields should be initialized */
  if ((res = updateRate(interface, config->slave)) != E_OK)
    return res;

  reg->DAO &= ~DAO_RESET;
  reg->DAI &= ~DAI_RESET;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2sDeinit(void *object)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);

  reg->IRQ = 0;
  reg->DMA1 = reg->DMA2 = 0;
  reg->RXRATE = reg->TXRATE = 0;

  if (interface->txDma)
    deinit(interface->txDma);
  if (interface->rxDma)
    deinit(interface->rxDma);
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
      return dmaStatus(interface->rxDma); //FIXME

    case IF_RX_CAPACITY:
      *(uint32_t *)data = BLOCK_COUNT - ((dmaCount(interface->rxDma) + 1) >> 1);
      return E_OK;

    case IF_TX_CAPACITY:
      *(uint32_t *)data = BLOCK_COUNT - ((dmaCount(interface->txDma) + 1) >> 1);
      return E_OK;

    case IF_WIDTH:
      *((uint32_t *)data) = 1 << (interface->width + 3);
      return E_OK;

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
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;
  const uint8_t sampleSizePow = interface->width + (interface->mono ? 0 : 1);
  const uint32_t samples = length >> sampleSizePow;

  if (!samples)
    return 0;

  /* Strict requirements on the buffer length */
  assert(samples > (interface->size >> 1) && samples <= interface->size);

  const bool ongoing = dmaStatus(interface->rxDma) == E_BUSY;

  if (!ongoing)
  {
    /* Clear FIFO */
    reg->DAI |= DAI_RESET;
  }

  /*
   * When the transfer is already active it will be continued.
   * 32-bit DMA transfers are used.
   */
  const enum result res = dmaStart(interface->rxDma, buffer,
      (const void *)&reg->RXFIFO, (samples << sampleSizePow) >> 2);

  if (res != E_OK)
    return 0;

  if (!ongoing)
  {
    reg->DMA1 |= DMA_RX_ENABLE;
    reg->DAI &= ~(DAI_STOP | DAI_RESET);
  }

  return samples << sampleSizePow;
}
/*----------------------------------------------------------------------------*/
static uint32_t i2sWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;
  const uint8_t sampleSizePow = interface->width + (interface->mono ? 0 : 1);
  const uint32_t samples = length >> sampleSizePow;

  if (!samples)
    return 0;

  /* Strict requirements on the buffer length */
  assert(samples > (interface->size >> 1) && samples <= interface->size);

  const bool ongoing = dmaStatus(interface->txDma) == E_BUSY;

  /*
   * When the transfer is already active it will be continued.
   * 32-bit DMA transfers are used.
   */
  const enum result res = dmaStart(interface->txDma, (void *)&reg->TXFIFO,
      buffer, (samples << sampleSizePow) >> 2);

  if (res != E_OK)
    return 0;

  if (!ongoing)
  {
    reg->DMA2 |= DMA_TX_ENABLE;
    reg->DAO &= ~DAO_STOP;
    reg->IRQ |= IRQ_TX_ENABLE;
  }

  return samples << sampleSizePow;
}
