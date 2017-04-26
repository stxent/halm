/*
 * i2s_dma.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gpdma_list.h>
#include <halm/platform/nxp/i2s_defs.h>
#include <halm/platform/nxp/i2s_dma.h>
/*----------------------------------------------------------------------------*/
#define BLOCK_COUNT 2
/*----------------------------------------------------------------------------*/
static enum result dmaSetup(struct I2sDma *, const struct I2sDmaConfig *,
    bool, bool);
static void interruptHandler(void *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static enum result updateRate(struct I2sDma *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *, const void *);
static void i2sDeinit(void *);
static enum result i2sCallback(void *, void (*)(void *), void *);
static enum result i2sGet(void *, enum ifOption, void *);
static enum result i2sSet(void *, enum ifOption, const void *);
static size_t i2sRead(void *, void *, size_t);
static size_t i2sWrite(void *, const void *, size_t);
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
  static const struct GpDmaSettings dmaSettings[] = {
      {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_WORD,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_WORD,
              .increment = true
          }
      },
      {
          .source = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_WORD,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_WORD,
              .increment = false
          }
      }
  };

  if (rx)
  {
    const struct GpDmaListConfig dmaConfig = {
        .number = BLOCK_COUNT << 1,
        .event = GPDMA_I2S0_REQ1 + (config->channel << 1),
        .type = GPDMA_TYPE_P2M,
        .channel = config->rx.dma,
        .silent = false
    };

    interface->rxDma = init(GpDmaList, &dmaConfig);
    if (!interface->rxDma)
      return E_ERROR;

    dmaConfigure(interface->rxDma, &dmaSettings[0]);
    dmaSetCallback(interface->rxDma, rxDmaHandler, interface);
  }
  else
    interface->rxDma = 0;

  if (tx)
  {
    const struct GpDmaListConfig dmaConfig = {
        .number = BLOCK_COUNT << 1,
        .event = GPDMA_I2S0_REQ2 + (config->channel << 1),
        .type = GPDMA_TYPE_M2P,
        .channel = config->tx.dma,
        .silent = false
    };

    interface->txDma = init(GpDmaList, &dmaConfig);
    if (!interface->txDma)
      return E_ERROR;

    dmaConfigure(interface->txDma, &dmaSettings[1]);
    dmaSetCallback(interface->txDma, txDmaHandler, interface);
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
    if (dmaStatus(interface->rxDma) != E_BUSY)
    {
      reg->DAI |= DAI_STOP;
      reg->DMA1 &= ~DMA_RX_ENABLE;
    }
    else if ((dmaPending(interface->rxDma) & 1) == 0)
    {
      event = true;
    }
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
    if (dmaStatus(interface->txDma) != E_BUSY)
    {
      /* Workaround to transmit last sample correctly */
      reg->TXFIFO = 0;

      reg->DMA2 &= ~DMA_TX_ENABLE;
    }
    else if ((dmaPending(interface->txDma) & 1) == 0)
    {
      event = true;
    }
  }

  if (event && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result updateRate(struct I2sDma *interface, uint32_t sampleRate)
{
  LPC_I2S_Type * const reg = interface->base.reg;
  struct I2sRateConfig rateConfig;
  uint32_t divisor;

  if (interface->slave)
  {
    rateConfig.x = rateConfig.y = 1;
    divisor = 1;
  }
  else
  {
    uint32_t bitrate = sampleRate * (1 << (interface->sampleSize + 3));
    uint32_t masterClock = sampleRate << 7;

    divisor = masterClock / bitrate;

    const enum result res = i2sCalcRate((struct I2sBase *)interface,
        masterClock, &rateConfig);

    if (res != E_OK)
      return res;
  }

  const uint32_t rate = RATE_X_DIVIDER(rateConfig.x)
      | RATE_Y_DIVIDER(rateConfig.y);

  if (interface->rxDma)
  {
    reg->RXBITRATE = divisor - 1;
    reg->RXRATE = rate;
  }
  if (interface->txDma)
  {
    reg->TXBITRATE = divisor - 1;
    reg->TXRATE = rate;
  }

  interface->sampleRate = sampleRate;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2sInit(void *object, const void *configBase)
{
  const struct I2sDmaConfig * const config = configBase;
  assert(config);

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

  assert(config->rate);
  assert(config->rx.sda || config->tx.sda);

  /* Call base class constructor */
  if ((res = I2sBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  assert(config->width <= I2S_WIDTH_32);

  interface->callback = 0;
  interface->sampleRate = 0;
  interface->sampleSize = config->width + (config->mono ? 0 : 1);
  interface->mono = config->mono;
  interface->slave = config->slave;

  res = dmaSetup(interface, config, config->rx.sda != 0, config->tx.sda != 0);
  if (res != E_OK)
    return res;

  static const unsigned int wordWidthMap[] = {
      WORDWIDTH_8BIT,
      WORDWIDTH_16BIT,
      WORDWIDTH_32BIT
  };
  const unsigned int halfPeriod = (1 << (config->width + 3)) - 1;
  const unsigned int wordWidth = wordWidthMap[config->width];

  uint32_t dai = DAI_WORDWIDTH(wordWidth) | DAI_WS_HALFPERIOD(halfPeriod);
  uint32_t dao = DAO_WORDWIDTH(wordWidth) | DAO_WS_HALFPERIOD(halfPeriod);

  LPC_I2S_Type * const reg = interface->base.reg;

  reg->IRQ = 0;
  reg->RXMODE = reg->TXMODE = 0;
  reg->RXRATE = reg->TXRATE = 0;
  reg->RXBITRATE = reg->TXBITRATE = 0;

  if (interface->rxDma)
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
    {
      reg->RXMODE |= RXMODE_RX4PIN;
    }
    else
    {
      assert(config->rx.ws != 0 && config->rx.sck != 0);
    }
  }
  else
  {
    /* Set default values */
    dai |= DAI_WS_SEL;
    reg->DMA1 = 0;
  }

  if (interface->txDma)
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
    {
      reg->TXMODE |= TXMODE_TX4PIN;
    }
    else
    {
      assert(config->tx.ws != 0 && config->tx.sck != 0);
    }

    reg->IRQ |= IRQ_TX_DEPTH(0);
  }
  else
  {
    /* Set default values */
    dao |= DAO_WS_SEL | DAO_MUTE;
    reg->DMA2 = 0;
  }

  reg->DAO = dao | DAO_RESET;
  reg->DAI = dai | DAI_RESET;

  /* All fields should be already initialized */
  if ((res = updateRate(interface, config->rate)) != E_OK)
    return res;

  reg->DAO = dao;
  reg->DAI = dai;

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
    case IF_AVAILABLE:
      if (!interface->rxDma)
        return E_INVALID;

      *(size_t *)data = BLOCK_COUNT - ((dmaPending(interface->rxDma) + 1) >> 1);
      return E_OK;

    case IF_PENDING:
      if (!interface->txDma)
        return E_INVALID;

      *(size_t *)data = (dmaPending(interface->txDma) + 1) >> 1;
      return E_OK;

    case IF_STATUS:
    {
      enum result res;

      if (interface->rxDma && (res = dmaStatus(interface->rxDma)) != E_OK)
        return res;
      if (interface->txDma && (res = dmaStatus(interface->txDma)) != E_OK)
        return res;

      return E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result i2sSet(void *object, enum ifOption option,
    const void *data)
{
  struct I2sDma * const interface = object;

  switch (option)
  {
    case IF_RATE:
      return updateRate(interface, *(const uint32_t *)data);

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t i2sRead(void *object, void *buffer, size_t length)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;
  const size_t samples = length >> interface->sampleSize;

  /* At least 2 samples */
  assert(samples >= 2);

  const size_t elements = (samples << interface->sampleSize) >> 2;
  const size_t parts[] = {elements / 2, elements - elements / 2};
  uint32_t *destination = buffer;

  /*
   * When the transfer is already active it will be continued.
   * 32-bit DMA transfers are used.
   */
  dmaAppend(interface->rxDma, destination, (const uint32_t *)&reg->RXFIFO,
      parts[0]);
  destination += parts[0];
  dmaAppend(interface->rxDma, destination, (const uint32_t *)&reg->RXFIFO,
      parts[1]);

  if (dmaStatus(interface->rxDma) != E_BUSY)
  {
    /* Clear internal FIFO */
    reg->DAI |= DAI_RESET;

    if (dmaEnable(interface->rxDma) != E_OK)
    {
      dmaClear(interface->txDma);
      return 0;
    }

    reg->DMA1 |= DMA_RX_ENABLE;
    reg->DAI &= ~(DAI_STOP | DAI_RESET);
  }

  return samples << interface->sampleSize;
}
/*----------------------------------------------------------------------------*/
static size_t i2sWrite(void *object, const void *buffer, size_t length)
{
  struct I2sDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;
  const size_t samples = length >> interface->sampleSize;

  /* At least 2 samples */
  assert(samples >= 2);

  const size_t elements = (samples << interface->sampleSize) >> 2;
  const size_t parts[] = {elements / 2, elements - elements / 2};
  const uint32_t *source = buffer;

  /*
   * When the transfer is already active it will be continued.
   * 32-bit DMA transfers are used.
   */
  dmaAppend(interface->txDma, (void *)&reg->TXFIFO, source, parts[0]);
  source += parts[0];
  dmaAppend(interface->txDma, (void *)&reg->TXFIFO, source, parts[1]);

  if (dmaStatus(interface->txDma) != E_BUSY)
  {
    if (dmaEnable(interface->txDma) != E_OK)
    {
      dmaClear(interface->txDma);
      return 0;
    }

    reg->DMA2 |= DMA_TX_ENABLE;
    reg->DAO &= ~DAO_STOP;
    reg->IRQ |= IRQ_TX_ENABLE;
  }

  return samples << interface->sampleSize;
}
