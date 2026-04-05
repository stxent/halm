/*
 * i2s_dma.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/pointer_queue.h>
#include <halm/platform/stm32/i2s_dma.h>
#include <halm/platform/stm32/spi_defs.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
struct I2SDmaStreamConfig
{
  /** Mandatory: pointer to a parent object. */
  struct I2SDma *parent;
  /** Mandatory: buffer size. */
  size_t depth;
  /** Mandatory: queue size. */
  size_t size;
};

struct I2SDmaStream
{
  struct Stream base;

  /* Parent interface */
  struct I2SDma *parent;
  /* Request queue */
  PointerQueue requests;
  /* Total capacity of all pending requests */
  size_t pending;
  /* Position inside first request */
  size_t position;
  /* Circular DMA buffer */
  uint8_t *buffer;
  /* Stop transfer flag */
  bool stop;
};
/*----------------------------------------------------------------------------*/
static void cleanupInterface(struct I2SDma *);
static bool dmaSetup(struct I2SDma *, const struct I2SDmaConfig *);
static void enqueueRxBuffer(void *);
static void enqueueTxBuffer(void *);
static void *getReceptionReg(struct I2SDma *);
static void modeSetup(struct I2SDma *, const struct I2SDmaConfig *);
static bool streamSetup(struct I2SDma *, const struct I2SDmaConfig *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);
static bool updateRate(struct I2SDma *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result i2sInit(void *, const void *);
static enum Result i2sGetParam(void *, int, void *);
static enum Result i2sSetParam(void *, int, const void *);

static enum Result i2sStreamInit(void *, const void *);
static void i2sRxStreamClear(void *);
static enum Result i2sRxStreamEnqueue(void *, struct StreamRequest *);
static void i2sTxStreamClear(void *);
static enum Result i2sTxStreamEnqueue(void *, struct StreamRequest *);

#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void i2sDeinit(void *);
static void i2sStreamDeinit(void *);
#else
#  define i2sDeinit deletedDestructorTrap
#  define i2sStreamDeinit deletedDestructorTrap
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

    .clear = i2sRxStreamClear,
    .enqueue = i2sRxStreamEnqueue
};

const struct StreamClass * const I2SDmaTxStream = &(const struct StreamClass){
    .size = sizeof(struct I2SDmaStream),
    .init = i2sStreamInit,
    .deinit = i2sStreamDeinit,

    .clear = i2sTxStreamClear,
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
  static const struct DmaSettings dmaSettings[] = {
      {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_HALFWORD,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_4,
              .width = DMA_WIDTH_HALFWORD,
              .increment = true
          }
      }, {
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
      }
  };
  const bool bidirectional = config->rx.sd && config->tx.sd;

  if (!bidirectional && config->rx.sd)
  {
    interface->rxDma = spiMakeCircularDma(interface->base.channel,
        config->rx.dma, config->priority, DMA_TYPE_P2M);
    if (interface->rxDma == NULL)
      return false;
  }
  else if (bidirectional)
  {
    interface->rxDma = i2sMakeCircularDma(interface->base.channel,
        config->rx.dma, config->priority, DMA_TYPE_P2M);
    if (interface->rxDma == NULL)
      return false;
  }
  if (interface->rxDma != NULL)
  {
    dmaConfigure(interface->rxDma, &dmaSettings[0]);
    dmaSetCallback(interface->rxDma, rxDmaHandler, interface);
  }

  if (config->tx.sd)
  {
    interface->txDma = spiMakeCircularDma(interface->base.channel,
        config->tx.dma, config->priority, DMA_TYPE_M2P);
    if (interface->txDma == NULL)
      return false;
  }
  if (interface->txDma != NULL)
  {
    dmaConfigure(interface->txDma, &dmaSettings[1]);
    dmaSetCallback(interface->txDma, txDmaHandler, interface);
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static void enqueueRxBuffer(void *object)
{
  struct I2SDma * const interface = object;
  struct I2SDmaStream * const stream = interface->rxStream;
  const IrqState state = irqSave();

  if (dmaStatus(interface->rxDma) != E_BUSY
      && stream->pending >= interface->bufferSize)
  {
    STM_SPI_Type * const reg = getReceptionReg(interface);
    const uint32_t * const src = (const uint32_t *)&reg->DR;

    stream->position = 0;

    dmaAppend(interface->rxDma, stream->buffer, src, interface->bufferSize);
    if (dmaEnable(interface->rxDma) == E_OK)
    {
      stream->stop = false;
      reg->CR2 |= CR2_RXDMAEN;
    }
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void enqueueTxBuffer(void *object)
{
  struct I2SDma * const interface = object;
  struct I2SDmaStream * const stream = interface->txStream;
  const IrqState state = irqSave();

  if (dmaStatus(interface->txDma) != E_BUSY
      && stream->pending >= interface->bufferSize)
  {
    size_t position = 0;

    while (!pointerQueueEmpty(&stream->requests))
    {
      struct StreamRequest * const current =
          pointerQueueFront(&stream->requests);
      const size_t bufferAvailable = interface->bufferSize - position;
      const size_t requestAvailable = current->length - stream->position;
      const size_t chunk = MIN(bufferAvailable, requestAvailable);
      const uint8_t * const buffer = current->buffer;

      memcpy(stream->buffer + position, buffer + stream->position, chunk);
      stream->pending -= chunk;
      stream->position += chunk;
      position += chunk;

      if (stream->position == current->length)
      {
        stream->position = 0;
        pointerQueuePopFront(&stream->requests);

        current->callback(current->argument, current, STREAM_REQUEST_COMPLETED);
      }

      if (position == interface->bufferSize)
      {
        STM_SPI_Type * const reg = interface->base.reg;
        uint32_t * const dst = (uint32_t *)&reg->DR;

        dmaAppend(interface->txDma, dst, stream->buffer, interface->bufferSize);
        if (dmaEnable(interface->txDma) == E_OK)
        {
          stream->stop = false;
          reg->CR2 |= CR2_TXDMAEN;
        }
        break;
      }
    }
  }

  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
static void *getReceptionReg(struct I2SDma *interface)
{
  return interface->fullduplex ?
      i2sGetExtension(&interface->base) : interface->base.reg;
}
/*----------------------------------------------------------------------------*/
static void modeSetup(struct I2SDma *interface,
    const struct I2SDmaConfig *config)
{
  STM_SPI_Type * const pri = interface->base.reg;
  STM_SPI_Type * const ext = i2sGetExtension(&interface->base);
  uint32_t cfgrPri = I2SCFGR_I2SMOD | I2SCFGR_I2SSTD(I2SSTD_I2S_PHILIPS);
  uint32_t pr = pri->I2SPR & ~I2SPR_MCKOE;

  if (config->mck)
    pr |= I2SPR_MCKOE;
  if (config->width == I2S_WIDTH_32)
    cfgrPri |= I2SCFGR_DATLEN(DATLEN_32BIT);

  uint32_t cfgrExt = cfgrPri;

  if (interface->fullduplex)
  {
    if (config->slave)
    {
      cfgrPri |= I2SCFGR_I2SCFG(I2SCFG_SLAVE_TX);
      cfgrExt |= I2SCFGR_I2SCFG(I2SCFG_SLAVE_RX);
    }
    else
    {
      cfgrPri |= I2SCFGR_I2SCFG(I2SCFG_MASTER_TX);
      cfgrExt |= I2SCFGR_I2SCFG(I2SCFG_MASTER_RX);
    }
  }
  else
  {
    if (config->rx.sd)
    {
      if (config->slave)
        cfgrPri |= I2SCFGR_I2SCFG(I2SCFG_SLAVE_RX);
      else
        cfgrPri |= I2SCFGR_I2SCFG(I2SCFG_MASTER_RX);
    }
    else
    {
      if (config->slave)
        cfgrPri |= I2SCFGR_I2SCFG(I2SCFG_SLAVE_TX);
      else
        cfgrPri |= I2SCFGR_I2SCFG(I2SCFG_MASTER_TX);
    }
  }

  pri->CR2 = 0;
  pri->I2SPR = pr;
  pri->I2SCFGR = cfgrPri;
  pri->I2SCFGR |= I2SCFGR_I2SE;

  if (interface->fullduplex)
  {
    ext->CR2 = 0;
    ext->I2SPR = pr;
    ext->I2SCFGR = cfgrExt;
    ext->I2SCFGR |= I2SCFGR_I2SE;
  }
}
/*----------------------------------------------------------------------------*/
static bool streamSetup(struct I2SDma *interface,
    const struct I2SDmaConfig *config)
{
  const struct I2SDmaStreamConfig streamConfig = {
      .parent = interface,
      .depth = config->depth,
      .size = config->size
  };

  if (config->rx.sd != 0)
  {
    interface->rxStream = init(I2SDmaRxStream, &streamConfig);
    if (interface->rxStream == NULL)
      return false;
  }

  if (config->tx.sd != 0)
  {
    interface->txStream = init(I2SDmaTxStream, &streamConfig);
    if (interface->txStream == NULL)
      return false;
  }

  return true;
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct I2SDma * const interface = object;
  struct I2SDmaStream * const stream = interface->rxStream;
  const size_t streamIndex = dmaQueued(interface->rxDma);

  assert(streamIndex >= 1 && streamIndex <= 2);
  assert(dmaStatus(interface->rxDma) == E_BUSY);

  if (!stream->stop)
  {
    const size_t count = interface->bufferSize >> 1;
    const size_t border = streamIndex == 1 ? count : interface->bufferSize;
    size_t position = border - count;

    if (stream->pending)
    {
      while (!pointerQueueEmpty(&stream->requests))
      {
        struct StreamRequest * const current =
            pointerQueueFront(&stream->requests);
        const size_t bufferAvailable = border - position;
        const size_t requestAvailable = current->capacity - stream->position;
        const size_t chunk = MIN(bufferAvailable, requestAvailable);
        uint8_t * const buffer = current->buffer;

        memcpy(buffer + stream->position, stream->buffer + position, chunk);
        stream->pending -= chunk;
        stream->position += chunk;
        position += chunk;

        if (stream->position == current->capacity)
        {
          stream->position = 0;
          pointerQueuePopFront(&stream->requests);

          current->length = current->capacity;
          current->callback(current->argument, current,
              STREAM_REQUEST_COMPLETED);
        }

        if (position == border)
          break;
      }
    }

    if (!stream->pending)
    {
      STM_SPI_Type * const reg = interface->base.reg;

      /* Request queue drained, stop the transfer */
      reg->CR2 &= ~CR2_RXDMAEN;

      dmaDisable(interface->rxDma);
      dmaClear(interface->rxDma);

      stream->stop = true;
    }
  }
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct I2SDma * const interface = object;
  struct I2SDmaStream * const stream = interface->txStream;
  const size_t streamIndex = dmaQueued(interface->txDma);
  
  assert(streamIndex >= 1 && streamIndex <= 2);
  assert(dmaStatus(interface->txDma) == E_BUSY);
  
  if (!stream->stop)
  {
    const size_t count = interface->bufferSize >> 1;
    const size_t border = streamIndex == 1 ? count : interface->bufferSize;
    size_t position = border - count;

    if (stream->pending >= count)
    {
      while (!pointerQueueEmpty(&stream->requests))
      {
        struct StreamRequest * const current =
            pointerQueueFront(&stream->requests);
        const size_t bufferAvailable = border - position;
        const size_t requestAvailable = current->length - stream->position;
        const size_t chunk = MIN(bufferAvailable, requestAvailable);
        const uint8_t * const buffer = current->buffer;

        memcpy(stream->buffer + position, buffer + stream->position, chunk);
        stream->pending -= chunk;
        stream->position += chunk;
        position += chunk;

        if (stream->position == current->length)
        {
          stream->position = 0;
          pointerQueuePopFront(&stream->requests);

          current->callback(current->argument, current,
              STREAM_REQUEST_COMPLETED);
        }

        if (position == border)
          break;
      }
    }
    else 
    {
      /* Mute and then disable on the next chunk completion */
      memset(stream->buffer + position, 0, count);
      stream->stop = true;

      if (!pointerQueueEmpty(&stream->requests))
      {
        struct StreamRequest * const current =
            pointerQueueFront(&stream->requests);

        stream->pending -= current->length - stream->position;
        stream->position = 0;
        pointerQueuePopFront(&stream->requests);

        current->callback(current->argument, current,
            STREAM_REQUEST_COMPLETED);
      }
    }
  }
  else
  {
    STM_SPI_Type * const reg = interface->base.reg;

    /* Sample queue drained, stop the transfer */
    reg->CR2 &= ~CR2_TXDMAEN;
    
    dmaDisable(interface->txDma);
    dmaClear(interface->txDma);

    /* Try to enqueue pending data */
    enqueueTxBuffer(interface);
  }
}
/*----------------------------------------------------------------------------*/
static bool updateRate(struct I2SDma *interface, uint32_t sampleRate)
{
  STM_SPI_Type * const pri = interface->base.reg;
  STM_SPI_Type * const ext = i2sGetExtension(&interface->base);
  const uint32_t enabled = pri->I2SCFGR & I2SCFGR_I2SE;
  uint32_t pr = pri->I2SPR & ~(I2SPR_I2SDIV_MASK | I2SPR_ODD);
  uint32_t bitrate = (8 << interface->sampleSize) * sampleRate;
  
  if (pr & I2SPR_MCKOE)
    bitrate *= 32 >> interface->sampleSize;

  const uint32_t frequency = i2sGetClock(&interface->base);
  const uint32_t divisor = frequency / bitrate;

  if (divisor < I2SPR_I2SDIV_MIN || divisor > I2SPR_I2SDIV_MAX)
    return false;
  
  pr |= I2SPR_I2SDIV(divisor / 2);
  if (divisor & 1)
    pr |= I2SPR_ODD;

  pri->I2SCFGR &= ~I2SCFGR_I2SE;
  pri->I2SPR = pr;
  pri->I2SCFGR |= enabled;

  if (interface->fullduplex)
  {
    ext->I2SCFGR &= ~I2SCFGR_I2SE;
    ext->I2SPR = pr;
    ext->I2SCFGR |= enabled;
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
  assert(config->rx.sd || config->tx.sd);
  assert(config->width <= I2S_WIDTH_32);
  assert(config->depth >= (size_t)(1 << (2 + config->width)) && config->size);

  const struct SpiBaseConfig baseConfig = {
      .cs = config->ws,
      .miso = config->rx.sd,
      .mosi = config->tx.sd,
      .sck = config->sck,
      .channel = config->channel,
      .extension = config->rx.sd && config->tx.sd,
      .slave = false
  };
  struct I2SDma * const interface = object;

  /* Call base class constructor */
  const enum Result res = SpiBase->init(interface, &baseConfig);
  if (res != E_OK)
    return res;

  /* Configure Master Clock output */
  if (config->mck)
    i2sConfigClockPin(config->channel, config->mck);

  interface->bufferSize = config->depth * 2;
  interface->sampleRate = 0;
  interface->sampleSize = 2 + config->width;
  interface->fullduplex = config->rx.sd && config->tx.sd;
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

  if (!updateRate(interface, config->rate))
  {
    cleanupInterface(interface);
    return E_VALUE;
  }

  modeSetup(interface, config);
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
      return updateRate(interface, *(const uint32_t *)data) ? E_OK : E_VALUE;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result i2sStreamInit(void *object, const void *configBase)
{
  const struct I2SDmaStreamConfig * const config = configBase;
  struct I2SDmaStream * const stream = object;

  stream->buffer = malloc(config->depth * 2);
  if (stream->buffer == NULL)
    return E_MEMORY;

  if (!pointerQueueInit(&stream->requests, config->size))
    return E_MEMORY;

  stream->parent = config->parent;
  stream->pending = 0;
  stream->position = 0;
  stream->stop = false;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2sRxStreamClear(void *object)
{
  struct I2SDmaStream * const stream = object;
  struct I2SDma * const interface = stream->parent;
  const IrqState state = irqSave();

  if (dmaStatus(interface->rxDma) == E_BUSY)
  {
    STM_SPI_Type * const reg = getReceptionReg(interface);

    reg->CR2 &= ~CR2_RXDMAEN;

    dmaDisable(interface->rxDma);
    dmaClear(interface->rxDma);
  }
  irqRestore(state);

  stream->pending = 0;
  stream->position = 0;

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

  assert(request != NULL && request->callback != NULL);
  /* Ensure the buffer has enough space and is aligned on the sample size */
  assert(request->capacity >> interface->sampleSize >= 1);
  assert(request->capacity % (1 << interface->sampleSize) == 0);

  const IrqState state = irqSave();
  enum Result res;

  if (!pointerQueueFull(&stream->requests))
  {
    pointerQueuePushBack(&stream->requests, request);
    stream->pending += request->capacity;
    res = E_OK;
  }
  else
    res = E_FULL;
  irqRestore(state);
  
  if (res == E_OK)
    enqueueRxBuffer(interface);
  return res;
}
/*----------------------------------------------------------------------------*/
static void i2sTxStreamClear(void *object)
{
  struct I2SDmaStream * const stream = object;
  struct I2SDma * const interface = stream->parent;
  const IrqState state = irqSave();

  if (dmaStatus(interface->txDma) == E_BUSY)
  {
    STM_SPI_Type * const reg = interface->base.reg;

    reg->CR2 &= ~CR2_TXDMAEN;

    dmaDisable(interface->txDma);
    dmaClear(interface->txDma);
  }
  irqRestore(state);

  stream->pending = 0;
  stream->position = 0;

  while (!pointerQueueEmpty(&stream->requests))
  {
    struct StreamRequest * const request = pointerQueueFront(&stream->requests);
    pointerQueuePopFront(&stream->requests);

    request->callback(request->argument, request, STREAM_REQUEST_CANCELLED);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result i2sTxStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  struct I2SDmaStream * const stream = object;
  struct I2SDma * const interface = stream->parent;

  assert(request != NULL && request->callback != NULL);
  /* Ensure the buffer has enough space and is aligned on the sample size */
  assert(request->length >> interface->sampleSize >= 1);
  assert(request->length % (1 << interface->sampleSize) == 0);

  const IrqState state = irqSave();
  enum Result res;
  
  if (!pointerQueueFull(&stream->requests))
  {
    pointerQueuePushBack(&stream->requests, request);
    stream->pending += request->length;
    res = E_OK;
  }
  else
    res = E_FULL;
  irqRestore(state);

  if (res == E_OK)
    enqueueTxBuffer(interface);
  return res;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void i2sDeinit(void *object)
{
  struct I2SDma * const interface = object;
  STM_SPI_Type * const reg = interface->base.reg;

  reg->I2SCFGR = 0;
  reg->CR2 = 0;

  cleanupInterface(interface);
  SpiBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_SPI_NO_DEINIT
static void i2sStreamDeinit(void *object)
{
  struct I2SDmaStream * const stream = object;

  pointerQueueDeinit(&stream->requests);
  free(stream->buffer);
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
