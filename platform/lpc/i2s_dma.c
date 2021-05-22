/*
 * i2s_dma.c
 * Copyright (C) 2015 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gpdma_list.h>
#include <halm/platform/lpc/i2s_defs.h>
#include <halm/platform/lpc/i2s_dma.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_COUNT 2
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct I2SDma *, const struct I2SDmaConfig *, bool, bool);
static void interruptHandler(void *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static enum Result updateRate(struct I2SDma *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result i2sInit(void *, const void *);
static void i2sSetCallback(void *, void (*)(void *), void *);
static enum Result i2sGetParam(void *, int, void *);
static enum Result i2sSetParam(void *, int, const void *);
static size_t i2sRead(void *, void *, size_t);
static size_t i2sWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_I2S_NO_DEINIT
static void i2sDeinit(void *);
#else
#define i2sDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const I2SDma = &(const struct InterfaceClass){
    .size = sizeof(struct I2SDma),
    .init = i2sInit,
    .deinit = i2sDeinit,

    .setCallback = i2sSetCallback,
    .getParam = i2sGetParam,
    .setParam = i2sSetParam,
    .read = i2sRead,
    .write = i2sWrite
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct I2SDma *interface,
    const struct I2SDmaConfig *config, bool rx, bool tx)
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
      }, {
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
        .number = BUFFER_COUNT << 1,
        .event = GPDMA_I2S0_REQ1 + (config->channel << 1),
        .type = GPDMA_TYPE_P2M,
        .channel = config->rx.dma
    };

    interface->rxDma = init(GpDmaList, &dmaConfig);
    if (!interface->rxDma)
      return false;

    dmaConfigure(interface->rxDma, &dmaSettings[0]);
    dmaSetCallback(interface->rxDma, rxDmaHandler, interface);
  }
  else
    interface->rxDma = 0;

  if (tx)
  {
    const struct GpDmaListConfig dmaConfig = {
        .number = BUFFER_COUNT << 1,
        .event = GPDMA_I2S0_REQ2 + (config->channel << 1),
        .type = GPDMA_TYPE_M2P,
        .channel = config->tx.dma
    };

    interface->txDma = init(GpDmaList, &dmaConfig);
    if (!interface->txDma)
      return false;

    dmaConfigure(interface->txDma, &dmaSettings[1]);
    dmaSetCallback(interface->txDma, txDmaHandler, interface);
  }
  else
    interface->txDma = 0;

  return true;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2SDma * const interface = object;
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
  struct I2SDma * const interface = object;
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
  struct I2SDma * const interface = object;
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
static enum Result updateRate(struct I2SDma *interface, uint32_t sampleRate)
{
  LPC_I2S_Type * const reg = interface->base.reg;
  struct I2SRateConfig rateConfig;
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

    const enum Result res = i2sCalcRate(&interface->base, masterClock,
        &rateConfig);

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
static enum Result i2sInit(void *object, const void *configBase)
{
  const struct I2SDmaConfig * const config = configBase;
  assert(config);

  const struct I2SBaseConfig baseConfig = {
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
  struct I2SDma * const interface = object;
  enum Result res;

  assert(config->rate);
  assert(config->rx.sda || config->tx.sda);
  assert(config->width <= I2S_WIDTH_32);

  /* Call base class constructor */
  if ((res = I2SBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = 0;
  interface->sampleRate = 0;
  interface->sampleSize = config->width + (config->mono ? 0 : 1);
  interface->mono = config->mono;
  interface->slave = config->slave;

  if (!dmaSetup(interface, config, config->rx.sda != 0, config->tx.sda != 0))
    return E_ERROR;

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
#ifndef CONFIG_PLATFORM_LPC_I2S_NO_DEINIT
static void i2sDeinit(void *object)
{
  struct I2SDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);

  reg->IRQ = 0;
  reg->DMA1 = reg->DMA2 = 0;
  reg->RXRATE = reg->TXRATE = 0;

  if (interface->txDma)
    deinit(interface->txDma);
  if (interface->rxDma)
    deinit(interface->rxDma);
  I2SBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void i2sSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2SDma * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result i2sGetParam(void *object, int parameter, void *data)
{
  struct I2SDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_AVAILABLE:
      if (!interface->rxDma)
        return E_INVALID;

      *(size_t *)data = BUFFER_COUNT - ((dmaPending(interface->rxDma) + 1) >> 1);
      return E_OK;

    case IF_PENDING:
      if (!interface->txDma)
        return E_INVALID;

      *(size_t *)data = (dmaPending(interface->txDma) + 1) >> 1;
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = interface->sampleRate;
      return E_OK;

    case IF_STATUS:
    {
      enum Result res;

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
static enum Result i2sSetParam(void *object, int parameter, const void *data)
{
  struct I2SDma * const interface = object;

  switch ((enum IfParameter)parameter)
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
  struct I2SDma * const interface = object;
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
  struct I2SDma * const interface = object;
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
