/*
 * ethernet.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/ethernet.h>
#include <halm/platform/lpc/lpc43xx/ethernet.h>
#include <halm/platform/lpc/lpc43xx/ethernet_defs.h>
#include <halm/platform/lpc/lpc43xx/ethernet_mdio.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
struct EthernetStreamConfig
{
  /** Mandatory: pointer to a parent object. */
  struct Ethernet *parent;
};

struct EthernetStream
{
  struct Stream base;

  /* Parent interface */
  struct Ethernet *parent;
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result ethInit(void *, const void *);
static void ethDeinit(void *);
static enum Result ethGetParam(void *, int, void *);
static enum Result ethSetParam(void *, int, const void *);

static enum Result ethStreamInit(void *, const void *);
static void ethRxStreamClear(void *);
static void ethTxStreamClear(void *);
static enum Result ethRxStreamEnqueue(void *, struct StreamRequest *);
static enum Result ethTxStreamEnqueue(void *, struct StreamRequest *);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Ethernet = &(const struct InterfaceClass){
    .size = sizeof(struct Ethernet),
    .init = ethInit,
    .deinit = ethDeinit,

    .setCallback = NULL,
    .getParam = ethGetParam,
    .setParam = ethSetParam,
    .read = NULL,
    .write = NULL
};

const struct StreamClass * const EthernetRxStream = &(const struct StreamClass){
    .size = sizeof(struct EthernetStream),
    .init = ethStreamInit,
    .deinit = NULL, /* Default destructor */

    .clear = ethRxStreamClear,
    .enqueue = ethRxStreamEnqueue
};

const struct StreamClass * const EthernetTxStream = &(const struct StreamClass){
    .size = sizeof(struct EthernetStream),
    .init = ethStreamInit,
    .deinit = NULL, /* Default destructor */

    .clear = ethTxStreamClear,
    .enqueue = ethTxStreamEnqueue
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Ethernet * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->base.reg;

  if (reg->DMA_STAT & DMA_STAT_RI)
  {
    while (interface->rxCount)
    {
      struct ReceiveDescriptor * const desc =
          interface->rxList + interface->rxIndex;

      if (desc->RDES0 & RDES0_OWN)
        break;

      ++interface->rxIndex;
      if (interface->rxIndex == interface->rxSize)
        interface->rxIndex = 0;
      --interface->rxCount;

      struct StreamRequest * const request =
          (struct StreamRequest *)desc->RDES5;

      request->length = RDES0_FL_VALUE(desc->RDES0);
      request->callback(request->argument, request, STREAM_REQUEST_COMPLETED);
    }

    /* Clear pending receive interrupt */
    reg->DMA_STAT = DMA_STAT_RI;

    if (interface->rxCount)
    {
      const uint32_t status = DMA_STAT_RS_VALUE(reg->DMA_STAT);

      if (status == RS_STOPPED || status == RS_SUSPENDED)
      {
        reg->DMA_OP_MODE &= ~DMA_OP_MODE_SR;
        reg->DMA_REC_DES_ADDR =
            (uint32_t)(interface->rxList + interface->rxIndex);
        reg->DMA_OP_MODE |= DMA_OP_MODE_SR;
      }
    }
  }

  if (reg->DMA_STAT & DMA_STAT_TI)
  {
    while (interface->txCount)
    {
      struct TransmitDescriptor * const desc =
          interface->txList + interface->txIndex;

      if (desc->TDES0 & TDES0_OWN)
        break;

      ++interface->txIndex;
      if (interface->txIndex == interface->txSize)
        interface->txIndex = 0;
      --interface->txCount;

      struct StreamRequest * const request =
          (struct StreamRequest *)desc->TDES5;

      request->callback(request->argument, request, STREAM_REQUEST_COMPLETED);
    }

    /* Clear pending receive interrupt */
    reg->DMA_STAT = DMA_STAT_TI;

    if (interface->txCount)
    {
      const uint32_t status = DMA_STAT_TS_VALUE(reg->DMA_STAT);

      if (status == TS_STOPPED || status == TS_SUSPENDED)
      {
        reg->DMA_OP_MODE &= ~DMA_OP_MODE_ST;
        reg->DMA_TRANS_DES_ADDR =
            (uint32_t)(interface->txList + interface->txIndex);
        reg->DMA_OP_MODE |= DMA_OP_MODE_ST;
      }
    }
  }

  reg->DMA_STAT = DMA_STAT_NIS;
}
/*----------------------------------------------------------------------------*/
static void initRxQueue(struct Ethernet *interface)
{
  for (size_t index = 0; index < interface->rxSize; ++index)
  {
    interface->rxList[index].RDES0 = 0;
    interface->rxList[index].RDES1 = RDES1_RCH;
    interface->rxList[index].RDES2 = 0;

    if (index < interface->rxSize - 1)
      interface->rxList[index].RDES3 = (uint32_t)&interface->rxList[index + 1];
    else
      interface->rxList[index].RDES3 = (uint32_t)&interface->rxList[0];

    interface->rxList[index].RDES4 = 0;
    interface->rxList[index].RDES5 = 0;
    interface->rxList[index].RDES6 = 0;
    interface->rxList[index].RDES7 = 0;
  }
}
/*----------------------------------------------------------------------------*/
static void initTxQueue(struct Ethernet *interface)
{
  for (size_t index = 0; index < interface->txSize; ++index)
  {
    interface->txList[index].TDES0 = TDES0_TCH;
    interface->txList[index].TDES1 = 0;
    interface->txList[index].TDES2 = 0;

    if (index < interface->txSize - 1)
      interface->txList[index].TDES3 = (uint32_t)&interface->txList[index + 1];
    else
      interface->txList[index].TDES3 = (uint32_t)&interface->txList[0];

    interface->txList[index].TDES4 = 0;
    interface->txList[index].TDES5 = 0;
    interface->txList[index].TDES6 = 0;
    interface->txList[index].TDES7 = 0;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result ethInit(void *object, const void *configBase)
{
  const struct EthernetConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rate == 10000000 || config->rate == 100000000);

  const struct EthernetBaseConfig baseConfig = {
      .txclk = config->txclk,
      .rxdv = config->rxdv,
      .txen = config->txen,
      .rxd = {
          config->rxd[0],
          config->rxd[1],
          config->rxd[2],
          config->rxd[3]
      },
      .txd = {
          config->txd[0],
          config->txd[1],
          config->txd[2],
          config->txd[3]
      },
      .col = config->col,
      .crs = config->crs,
      .rxclk = config->rxclk,
      .rxer = config->rxer,
      .txer = config->txer,
      .mdc = config->mdc,
      .mdio = config->mdio
  };
  const struct EthernetStreamConfig streamConfig = {
      .parent = object
  };

  struct Ethernet * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = EthernetBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->rxList =
      malloc(sizeof(struct ReceiveDescriptor) * config->rxSize);
  if (interface->rxList == NULL)
    return E_MEMORY;

  interface->rxSize = config->rxSize;
  interface->rxCount = 0;
  interface->rxIndex = 0;
  initRxQueue(interface);

  interface->txList =
      malloc(sizeof(struct TransmitDescriptor) * config->txSize);
  if (interface->txList == NULL)
    return E_MEMORY;

  interface->txSize = config->txSize;
  interface->txCount = 0;
  interface->txIndex = 0;
  initTxQueue(interface);

  interface->rxStream = init(EthernetRxStream, &streamConfig);
  if (interface->rxStream == NULL)
    return E_ERROR;

  interface->txStream = init(EthernetTxStream, &streamConfig);
  if (interface->txStream == NULL)
    return E_ERROR;

  interface->base.handler = interruptHandler;

  /* Configure peripheral registers */

  LPC_ETHERNET_Type * const reg = interface->base.reg;

  /*
   * Reset all MAC internal registers and logic. The reset operation
   * is completed only when all the resets in all the active clock domains
   * are cleared. All the PHY clocks must be present for reset completion.
   */
  reg->DMA_BUS_MODE |= DMA_BUS_MODE_SWR;
  while (reg->DMA_BUS_MODE & DMA_BUS_MODE_SWR);

  /* Set the Ethernet MAC Address registers */
  reg->MAC_ADDR0_HIGH = (uint32_t)(config->address >> 32) | MAC_ADDR0_HIGH_MO;
  reg->MAC_ADDR0_LOW = (uint32_t)config->address;

  if (!config->halfduplex)
  {
    /* Enable full duplex mode */
    reg->MAC_CONFIG |= MAC_CONFIG_DM;
  }

  if (config->rate == 100000000)
  {
    /* Enable 100 Mbit/s mode */
    reg->MAC_CONFIG |= MAC_CONFIG_FES;
  }

  /* Configure frame filtering */
  reg->MAC_FRAME_FILTER = MAC_FILTER_PR | MAC_FILTER_RA;
  /* Enable Receiver and Transmitter */
  reg->MAC_CONFIG |= MAC_CONFIG_RE | MAC_CONFIG_TE;

  /* Clear pending interrupt flags */
  reg->DMA_STAT = 0xFFFFFFFFUL;
  /* Disable PMT and TS interrupts */
  reg->MAC_INTR_MASK = MAC_INTR_MASK_PMTIM | MAC_INTR_MASK_TSIM;
  /* Enable Receiver and Transmitter interrupts */
  reg->DMA_INT_EN = DMA_INT_EN_TIE | DMA_INT_EN_RIE | DMA_INT_EN_NIE;

  irqSetPriority(interface->base.irq, config->priority);
  irqClearPending(interface->base.irq);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void ethDeinit(void *object)
{
  struct Ethernet * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->base.reg;

  /* Disable interrupts */
  irqDisable(interface->base.irq);
  /* Disable Receiver and Transmitter */
  reg->MAC_CONFIG &= ~(MAC_CONFIG_RE | MAC_CONFIG_TE);

  free(interface->txList);
  free(interface->rxList);

  deinit(interface->txStream);
  deinit(interface->rxStream);

  EthernetBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum Result ethGetParam(void *object, int parameter, void *data)
{
  struct Ethernet * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS_64:
    {
      const uint32_t high = MAC_ADDR0_HIGH_ADDRESS_VALUE(reg->MAC_ADDR0_HIGH);
      const uint32_t low = reg->MAC_ADDR0_LOW;

      *(uint64_t *)data = ((uint64_t)high << 32) | low;
      return E_OK;
    }

    case IF_RATE:
    {
      const uint32_t rate = *(const uint32_t *)data;

      if (rate == 10000000)
      {
        /* Enable 10 Mbit/s mode */
        reg->MAC_CONFIG &= ~MAC_CONFIG_FES;
        return E_OK;
      }
      else if (rate == 100000000)
      {
        /* Enable 100 Mbit/s mode */
        reg->MAC_CONFIG |= MAC_CONFIG_FES;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result ethSetParam(void *object, int parameter, const void *data)
{
  struct Ethernet * const interface = object;
  LPC_ETHERNET_Type * const reg = interface->base.reg;

  switch ((enum EthParameter)parameter)
  {
    case IF_ETH_FULL_DUPLEX:
      reg->MAC_CONFIG |= MAC_CONFIG_DM;
      return E_OK;

    case IF_ETH_HALF_DUPLEX:
      reg->MAC_CONFIG &= ~MAC_CONFIG_DM;
      return E_OK;

    case IF_ETH_10M:
      reg->MAC_CONFIG &= ~MAC_CONFIG_FES;
      return E_OK;

    case IF_ETH_100M:
      reg->MAC_CONFIG |= MAC_CONFIG_FES;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS_64:
    {
      const uint64_t address = *(const uint64_t *)data;

      reg->MAC_ADDR0_HIGH = (uint32_t)(address >> 32) | MAC_ADDR0_HIGH_MO;
      reg->MAC_ADDR0_LOW = (uint32_t)address;

      return E_OK;
    }

    case IF_RATE:
    {
      const uint32_t rate = *(const uint32_t *)data;

      if (rate == 10000000)
      {
        /* Enable 10 Mbit/s mode */
        reg->MAC_CONFIG &= ~MAC_CONFIG_FES;
        return E_OK;
      }
      else if (rate == 100000000)
      {
        /* Enable 100 Mbit/s mode */
        reg->MAC_CONFIG |= MAC_CONFIG_FES;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result ethStreamInit(void *object, const void *configBase)
{
  const struct EthernetStreamConfig * const config = configBase;
  struct EthernetStream * const stream = object;

  stream->parent = config->parent;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void ethRxStreamClear(void *object)
{
  struct EthernetStream * const stream = object;
  struct Ethernet * const interface = stream->parent;

  // TODO
  (void)interface;
}
/*----------------------------------------------------------------------------*/
static void ethTxStreamClear(void *object)
{
  struct EthernetStream * const stream = object;
  struct Ethernet * const interface = stream->parent;

  // TODO
  (void)interface;
}
/*----------------------------------------------------------------------------*/
static enum Result ethRxStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  struct EthernetStream * const stream = object;
  struct Ethernet * const interface = stream->parent;
  const IrqState state = irqSave();

  if (interface->rxCount == interface->rxSize)
  {
    irqRestore(state);
    return E_FULL;
  }

  size_t idx = interface->rxIndex + interface->rxCount;
  if (idx >= interface->rxSize)
    idx -= interface->rxSize;

  LPC_ETHERNET_Type * const reg = interface->base.reg;
  struct ReceiveDescriptor * const desc = interface->rxList + idx;

  ++interface->rxCount;

  desc->RDES7 = 0;
  desc->RDES6 = 0;
  desc->RDES4 = 0;

  desc->RDES5 = (uint32_t)request;
  desc->RDES2 = (uint32_t)request->buffer;
  desc->RDES1 = RDES1_RCH | RDES1_RBS1(request->capacity);
  desc->RDES0 = RDES0_OWN;

  // TODO Receive poll demand
  const uint32_t status = DMA_STAT_RS_VALUE(reg->DMA_STAT);

  if (status == RS_STOPPED || status == RS_SUSPENDED)
  {
    reg->DMA_OP_MODE &= ~DMA_OP_MODE_SR;
    reg->DMA_REC_DES_ADDR =
        (uint32_t)(interface->rxList + interface->rxIndex);
    reg->DMA_OP_MODE |= DMA_OP_MODE_SR;
  }

  irqRestore(state);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result ethTxStreamEnqueue(void *object,
    struct StreamRequest *request)
{
  struct EthernetStream * const stream = object;
  struct Ethernet * const interface = stream->parent;
  const IrqState state = irqSave();

  if (interface->txCount == interface->txSize)
  {
    irqRestore(state);
    return E_FULL;
  }

  size_t idx = interface->txIndex + interface->txCount;
  if (idx >= interface->txSize)
    idx -= interface->txSize;

  LPC_ETHERNET_Type * const reg = interface->base.reg;
  struct TransmitDescriptor * const desc = interface->txList + idx;

  ++interface->txCount;

  desc->TDES7 = 0;
  desc->TDES6 = 0;
  desc->TDES4 = 0;

  desc->TDES5 = (uint32_t)request;
  desc->TDES2 = (uint32_t)request->buffer;
  desc->TDES1 = TDES1_TBS1(request->length);
  desc->TDES0 = TDES0_TCH | TDES0_IC | TDES0_OWN;
  desc->TDES0 |= TDES0_FS | TDES0_LS;
  // TODO TDES0_TER

  // TODO Transmit poll demand
  const uint32_t status = DMA_STAT_TS_VALUE(reg->DMA_STAT);

  if (status == TS_STOPPED || status == TS_SUSPENDED)
  {
    reg->DMA_OP_MODE &= ~DMA_OP_MODE_ST;
    reg->DMA_TRANS_DES_ADDR =
        (uint32_t)(interface->txList + interface->txIndex);
    reg->DMA_OP_MODE |= DMA_OP_MODE_ST;
  }

  irqRestore(state);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
struct Stream *ethGetInput(struct Ethernet *interface)
{
  return (struct Stream *)interface->rxStream;
}
/*----------------------------------------------------------------------------*/
struct Stream *ethGetOutput(struct Ethernet *interface)
{
  return (struct Stream *)interface->txStream;
}
/*----------------------------------------------------------------------------*/
struct Interface *ethMakeMDIO(struct Ethernet *interface)
{
  const struct MDIOConfig config = {
      .parent = interface
  };

  return init(MDIO, &config);
}
