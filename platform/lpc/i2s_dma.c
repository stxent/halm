/*
 * i2s_dma.c
 * Copyright (C) 2015, 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/gpdma_list.h>
#include <halm/platform/lpc/i2s_defs.h>
#include <halm/platform/lpc/i2s_dma.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct I2SDmaStreamConfig
{
  /** Mandatory: pointer to a parent object. */
  struct I2SDma *parent;
  /** Mandatory: queue size. */
  size_t size;
};

struct I2SDmaStream
{
  struct Stream base;

  /* Parent interface */
  struct I2SDma *parent;
  /* Queued requests */
  PointerQueue requests;
};
/*----------------------------------------------------------------------------*/
static void cleanupInterface(struct I2SDma *);
static bool dmaSetup(struct I2SDma *, const struct I2SDmaConfig *);
static void modeSetup(struct I2SDma *, const struct I2SDmaConfig *);
static bool streamSetup(struct I2SDma *, const struct I2SDmaConfig *);
static void interruptHandler(void *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static bool updateRate(struct I2SDma *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result i2sInit(void *, const void *);
static enum Result i2sGetParam(void *, int, void *);
static enum Result i2sSetParam(void *, int, const void *);

static enum Result i2sStreamInit(void *, const void *);
static void i2sStreamClear(void *);
static enum Result i2sRxStreamEnqueue(void *, struct StreamRequest *);
static enum Result i2sTxStreamEnqueue(void *, struct StreamRequest *);

#ifndef CONFIG_PLATFORM_LPC_I2S_NO_DEINIT
static void i2sDeinit(void *);
static void i2sStreamDeinit(void *);
#else
#define i2sDeinit deletedDestructorTrap
#define i2sStreamDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const I2SDma = &(const struct InterfaceClass){
    .size = sizeof(struct I2SDma),
    .init = i2sInit,
    .deinit = i2sDeinit,

    .setCallback = NULL,
    .getParam = i2sGetParam,
    .setParam = i2sSetParam,
    .read = NULL,
    .write = NULL
};

const struct StreamClass * const I2SDmaRxStream = &(const struct StreamClass){
    .size = sizeof(struct I2SDmaStream),
    .init = i2sStreamInit,
    .deinit = i2sStreamDeinit,

    .clear = i2sStreamClear,
    .enqueue = i2sRxStreamEnqueue
};

const struct StreamClass * const I2SDmaTxStream = &(const struct StreamClass){
    .size = sizeof(struct I2SDmaStream),
    .init = i2sStreamInit,
    .deinit = i2sStreamDeinit,

    .clear = i2sStreamClear,
    .enqueue = i2sTxStreamEnqueue
};
/*----------------------------------------------------------------------------*/
static void cleanupInterface(struct I2SDma *interface)
{
  if (interface->txStream != NULL)
    deinit(interface->txStream);
  if (interface->rxStream != NULL)
    deinit(interface->rxStream);
  if (interface->txDma != NULL)
    deinit(interface->txDma);
  if (interface->rxDma != NULL)
    deinit(interface->rxDma);
}
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct I2SDma *interface,
    const struct I2SDmaConfig *config)
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

  if (config->rx.sda != 0)
  {
    const struct GpDmaListConfig dmaConfig = {
        .number = config->size << 1,
        .event = GPDMA_I2S0_REQ1 + (config->channel << 1),
        .type = GPDMA_TYPE_P2M,
        .channel = config->rx.dma
    };

    interface->rxDma = init(GpDmaList, &dmaConfig);
    if (interface->rxDma == NULL)
      return false;

    dmaConfigure(interface->rxDma, &dmaSettings[0]);
    dmaSetCallback(interface->rxDma, rxDmaHandler, interface);
  }

  if (config->tx.sda != 0)
  {
    const struct GpDmaListConfig dmaConfig = {
        .number = config->size << 1,
        .event = GPDMA_I2S0_REQ2 + (config->channel << 1),
        .type = GPDMA_TYPE_M2P,
        .channel = config->tx.dma
    };

    interface->txDma = init(GpDmaList, &dmaConfig);
    if (interface->txDma == NULL)
      return false;

    dmaConfigure(interface->txDma, &dmaSettings[1]);
    dmaSetCallback(interface->txDma, txDmaHandler, interface);
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static void modeSetup(struct I2SDma *interface,
    const struct I2SDmaConfig *config)
{
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
  reg->RXMODE = 0;
  reg->TXMODE = 0;
  reg->RXRATE = 0;
  reg->TXRATE = 0;
  reg->RXBITRATE = 0;
  reg->TXBITRATE = 0;

  if (config->rx.sda != 0)
  {
    assert((config->rx.ws != 0 && config->rx.sck != 0)
        || (config->rx.ws == 0 && config->rx.sck == 0));

    dai |= DAI_STOP;
    reg->DMA1 = DMA_RX_DEPTH(4);

    if (interface->mono)
      dai |= DAI_MONO;

    if (config->slave)
      dai |= DAI_WS_SEL;
    if (config->rx.mclk)
      reg->RXMODE |= RXMODE_RXMCENA;
    if (!config->rx.ws && !config->rx.sck)
      reg->RXMODE |= RXMODE_RX4PIN;
  }
  else
  {
    /* Set default values */
    dai |= DAI_WS_SEL;
    reg->DMA1 = 0;
  }

  if (config->tx.sda != 0)
  {
    assert((config->tx.ws != 0 && config->tx.sck != 0)
        || (config->tx.ws == 0 && config->tx.sck == 0));

    dao |= DAO_STOP;
    reg->DMA2 = DMA_TX_DEPTH(3);

    if (interface->mono)
      dao |= DAO_MONO;

    if (config->slave)
      dao |= DAO_WS_SEL;
    if (config->tx.mclk)
      reg->TXMODE |= TXMODE_TXMCENA;
    if (!config->tx.ws && !config->tx.sck)
      reg->TXMODE |= TXMODE_TX4PIN;

    reg->IRQ |= IRQ_TX_DEPTH(0);
  }
  else
  {
    /* Set default values */
    dao |= DAO_WS_SEL | DAO_MUTE;
    reg->DMA2 = 0;
  }

  reg->DAO = dao | DAO_RESET;
  reg->DAO &= ~DAO_RESET;

  reg->DAI = dai | DAI_RESET;
  reg->DAI &= ~DAI_RESET;
}
/*----------------------------------------------------------------------------*/
static bool streamSetup(struct I2SDma *interface,
    const struct I2SDmaConfig *config)
{
  const struct I2SDmaStreamConfig streamConfig = {
      .parent = interface,
      .size = config->size
  };

  if (config->rx.sda != 0)
  {
    interface->rxStream = init(I2SDmaRxStream, &streamConfig);
    if (interface->rxStream == NULL)
      return false;
  }

  if (config->tx.sda != 0)
  {
    interface->txStream = init(I2SDmaTxStream, &streamConfig);
    if (interface->txStream == NULL)
      return false;
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2SDma * const interface = object;
  LPC_I2S_Type * const reg = interface->base.reg;

  if (!(reg->DMA2 & DMA_TX_ENABLE))
    reg->DAO |= DAO_STOP;

  reg->IRQ &= ~IRQ_TX_ENABLE;
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct I2SDma * const interface = object;
  const enum Result transferStatus = dmaStatus(interface->rxDma);
  enum StreamRequestStatus requestStatus = STREAM_REQUEST_COMPLETED;
  bool event = false;

  if (transferStatus != E_BUSY)
  {
    LPC_I2S_Type * const reg = interface->base.reg;

    reg->DAI |= DAI_STOP;
    reg->DMA1 &= ~DMA_RX_ENABLE;

    if (transferStatus == E_ERROR)
      requestStatus = STREAM_REQUEST_FAILED;

    event = true;
  }
  else if ((dmaQueued(interface->rxDma) & 1) == 0)
  {
    /*
     * Each block consists of two buffers. Call user function
     * at the end of the odd block or at the end of the list.
     */
    event = true;
  }

  if (event)
  {
    struct StreamRequest * const request =
        pointerQueueFront(&interface->rxStream->requests);
    pointerQueuePopFront(&interface->rxStream->requests);

    if (requestStatus == STREAM_REQUEST_COMPLETED)
      request->length = request->capacity;
    request->callback(request->argument, request, requestStatus);
  }
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct I2SDma * const interface = object;
  const enum Result transferStatus = dmaStatus(interface->txDma);
  enum StreamRequestStatus requestStatus = STREAM_REQUEST_COMPLETED;
  bool event = false;

  if (transferStatus != E_BUSY)
  {
    LPC_I2S_Type * const reg = interface->base.reg;

    reg->DMA2 &= ~DMA_TX_ENABLE;

    /* Workaround to transmit last sample correctly */
    reg->IRQ |= IRQ_TX_ENABLE;

    if (transferStatus == E_ERROR)
      requestStatus = STREAM_REQUEST_FAILED;

    event = true;
  }
  else if ((dmaQueued(interface->txDma) & 1) == 0)
  {
    event = true;
  }

  if (event)
  {
    struct StreamRequest * const request =
        pointerQueueFront(&interface->txStream->requests);
    pointerQueuePopFront(&interface->txStream->requests);

    request->callback(request->argument, request, requestStatus);
  }
}
/*----------------------------------------------------------------------------*/
static bool updateRate(struct I2SDma *interface, uint32_t sampleRate)
{
  LPC_I2S_Type * const reg = interface->base.reg;
  struct I2SRateConfig rateConfig;
  uint32_t divisor;

  if (interface->slave)
  {
    rateConfig.x = 1;
    rateConfig.y = 1;
    divisor = 1;
  }
  else
  {
    const uint32_t masterClock = sampleRate * CONFIG_PLATFORM_LPC_I2S_FS;

    if (!i2sCalcRate(&interface->base, masterClock, &rateConfig))
      return false;

    divisor = CONFIG_PLATFORM_LPC_I2S_FS >> (interface->sampleSize + 3);
  }

  if (divisor == 0 || divisor >= BITRATE_DIVIDER_MASK)
    return false;

  const uint32_t clockRate = RATE_X_DIVIDER(rateConfig.x)
      | RATE_Y_DIVIDER(rateConfig.y);
  const uint32_t clockBitRate = divisor - 1;

  if (interface->rxDma != NULL)
  {
    reg->RXBITRATE = clockBitRate;
    reg->RXRATE = clockRate;
  }
  if (interface->txDma != NULL)
  {
    reg->TXBITRATE = clockBitRate;
    reg->TXRATE = clockRate;
  }

  interface->sampleRate = sampleRate;
  return true;
}
/*----------------------------------------------------------------------------*/
static enum Result i2sInit(void *object, const void *configBase)
{
  const struct I2SDmaConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rate);
  assert(config->rx.sda || config->tx.sda);
  assert(config->width <= I2S_WIDTH_32);

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

  /* Call base class constructor */
  const enum Result res = I2SBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->sampleRate = 0;
  interface->sampleSize = config->width + (config->mono ? 0 : 1);
  interface->mono = config->mono;
  interface->slave = config->slave;

  interface->rxDma = NULL;
  interface->txDma = NULL;
  interface->rxStream = NULL;
  interface->txStream = NULL;

  if (!streamSetup(interface, config))
  {
    cleanupInterface(interface);
    return E_ERROR;
  }

  if (!dmaSetup(interface, config))
  {
    cleanupInterface(interface);
    return E_ERROR;
  }

  modeSetup(interface, config);

  if (!updateRate(interface, config->rate))
  {
    cleanupInterface(interface);
    return E_VALUE;
  }

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result i2sGetParam(void *object, int parameter, void *data)
{
  struct I2SDma * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      if (interface->rxStream == NULL)
        return E_INVALID;

      *(size_t *)data = pointerQueueSize(&interface->rxStream->requests);
      return E_OK;

    case IF_RX_PENDING:
      if (interface->rxStream == NULL)
        return E_INVALID;

      *(size_t *)data = pointerQueueCapacity(&interface->rxStream->requests)
          - pointerQueueSize(&interface->rxStream->requests);
      return E_OK;

    case IF_TX_AVAILABLE:
      if (interface->txStream == NULL)
        return E_INVALID;

      *(size_t *)data = pointerQueueCapacity(&interface->txStream->requests)
          - pointerQueueSize(&interface->txStream->requests);
      return E_OK;

    case IF_TX_PENDING:
      if (interface->txStream == NULL)
        return E_INVALID;

      *(size_t *)data = pointerQueueSize(&interface->txStream->requests);
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = interface->sampleRate;
      return E_OK;

    case IF_STATUS:
    {
      enum Result res;

      if (interface->rxDma != NULL
          && (res = dmaStatus(interface->rxDma)) != E_OK)
      {
        return res;
      }
      if (interface->txDma != NULL
          && (res = dmaStatus(interface->txDma)) != E_OK)
      {
        return res;
      }

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
      return updateRate(interface, *(const uint32_t *)data) ? E_OK : E_ERROR;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result i2sStreamInit(void *object, const void *configBase)
{
  const struct I2SDmaStreamConfig * const config = configBase;
  struct I2SDmaStream * const stream = object;

  if (pointerQueueInit(&stream->requests, config->size))
  {
    stream->parent = config->parent;
    return E_OK;
  }
  else
    return E_MEMORY;
}
/*----------------------------------------------------------------------------*/
static void i2sStreamClear(void *object)
{
  struct I2SDmaStream * const stream = object;

  while (!pointerQueueEmpty(&stream->requests))
  {
    struct StreamRequest * const request = pointerQueueFront(&stream->requests);
    pointerQueuePopFront(&stream->requests);

    request->callback(request->argument, request, STREAM_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result i2sRxStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  struct I2SDmaStream * const stream = object;
  struct I2SDma * const interface = stream->parent;
  const size_t samples = request->capacity >> interface->sampleSize;

  assert(request != NULL && request->callback != NULL);
  /* Ensure the buffer has enough space and is aligned on the sample size */
  assert(request->capacity >> interface->sampleSize >= 2);
  assert(request->capacity % (1 << interface->sampleSize) == 0);
  /* Input buffer should be aligned on the burst size of DMA transfer */
  assert((uintptr_t)request->buffer % 16 == 0);

  const size_t elements = (samples << interface->sampleSize) >> 2;
  const size_t parts[] = {elements / 2, elements - elements / 2};

  enum Result res = E_OK;
  const IrqState state = irqSave();

  if (!pointerQueueFull(&stream->requests))
  {
    LPC_I2S_Type * const reg = interface->base.reg;
    const uint32_t * const src = (const uint32_t *)&reg->RXFIFO;
    uint32_t * const dst = request->buffer;

    /*
     * When the transfer is already active it will be continued.
     * 32-bit DMA transfers are used.
     */
    dmaAppend(interface->rxDma, dst, src, parts[0]);
    dmaAppend(interface->rxDma, dst + parts[0], src, parts[1]);

    if (dmaStatus(interface->rxDma) != E_BUSY)
    {
      /* Clear internal FIFO */
      reg->DAI |= DAI_RESET;

      if (dmaEnable(interface->rxDma) == E_OK)
      {
        reg->DMA1 |= DMA_RX_ENABLE;
        reg->DAI &= ~(DAI_STOP | DAI_RESET);
      }
      else
        res = E_INTERFACE;
    }
  }
  else
    res = E_FULL;

  if (res == E_OK)
    pointerQueuePushBack(&stream->requests, request);

  irqRestore(state);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result i2sTxStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  struct I2SDmaStream * const stream = object;
  struct I2SDma * const interface = stream->parent;

  assert(request != NULL && request->callback != NULL);
  /* Ensure the buffer has enough space and is aligned on the sample size */
  assert(request->capacity >> interface->sampleSize >= 2);
  assert(request->capacity % (1 << interface->sampleSize) == 0);
  /* Input buffer should be aligned on the burst size of DMA transfer */
  assert((uintptr_t)request->buffer % 16 == 0);

  const size_t words = request->length >> 2;
  const size_t parts[] = {words >> 1, words - (words >> 1)};

  enum Result res = E_OK;
  const IrqState state = irqSave();

  if (!pointerQueueFull(&stream->requests))
  {
    LPC_I2S_Type * const reg = interface->base.reg;
    uint32_t * const dst = (uint32_t *)&reg->TXFIFO;
    const uint32_t * const src = request->buffer;

    /*
     * When the transfer is already active it will be continued.
     * 32-bit DMA transfers are used.
     */
    dmaAppend(interface->txDma, dst, src, parts[0]);
    dmaAppend(interface->txDma, dst, src + parts[0], parts[1]);

    if (dmaStatus(interface->txDma) != E_BUSY)
    {
      if (dmaEnable(interface->txDma) == E_OK)
      {
        reg->DMA2 |= DMA_TX_ENABLE;
        reg->DAO &= ~DAO_STOP;
      }
      else
        res = E_INTERFACE;
    }
  }
  else
    res = E_FULL;

  if (res == E_OK)
    pointerQueuePushBack(&stream->requests, request);

  irqRestore(state);
  return res;
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

  cleanupInterface(interface);
  I2SBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_I2S_NO_DEINIT
static void i2sStreamDeinit(void *object)
{
  struct I2SDmaStream * const stream = object;
  pointerQueueDeinit(&stream->requests);
}
#endif
/*----------------------------------------------------------------------------*/
struct Stream *i2sDmaGetInput(struct I2SDma *interface)
{
  return (struct Stream *)interface->rxStream;
}
/*----------------------------------------------------------------------------*/
struct Stream *i2sDmaGetOutput(struct I2SDma *interface)
{
  return (struct Stream *)interface->txStream;
}
