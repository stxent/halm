/*
 * can.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <halm/generic/can.h>
#include <halm/platform/nxp/can.h>
#include <halm/platform/nxp/gen_2/can_defs.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
enum Mode
{
  MODE_LISTENER,
  MODE_ACTIVE,
  MODE_LOOPBACK
};
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY 50000000
#define MAX_MESSAGES  32

#define RX_OBJECT     1
#define TX_OBJECT     (MAX_MESSAGES / 2)
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(const struct Can *, uint32_t);
static void changeMode(struct Can *, enum Mode);
static void changeRate(struct Can *, uint32_t);
static void interruptHandler(void *);
static void invalidateMessage(struct Can *, unsigned int);
static void listenMessage(struct Can *, unsigned int);
static void readMessage(struct Can *, struct CanMessage *, unsigned int);
static void sendMessage(struct Can *, const struct CanMessage *, unsigned int,
    bool);
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);
static void canDeinit(void *);
static enum Result canSetCallback(void *, void (*)(void *), void *);
static enum Result canGetParam(void *, enum IfParameter, void *);
static enum Result canSetParam(void *, enum IfParameter, const void *);
static size_t canRead(void *, void *, size_t);
static size_t canWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass canTable = {
    .size = sizeof(struct Can),
    .init = canInit,
    .deinit = canDeinit,

    .setCallback = canSetCallback,
    .getParam = canGetParam,
    .setParam = canSetParam,
    .read = canRead,
    .write = canWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Can = &canTable;
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(const struct Can *interface, uint32_t rate)
{
  assert(rate != 0);

  const LPC_CAN_Type * const reg = interface->base.reg;

  const unsigned int bitsPerFrame = 1 + CONFIG_PLATFORM_NXP_CAN_TSEG1
      + CONFIG_PLATFORM_NXP_CAN_TSEG2;
  const uint32_t clock = canGetClock(&interface->base) / (reg->CLKDIV + 1);
  const uint32_t prescaler = clock / rate / bitsPerFrame;
  const uint32_t regValue = BT_BRP(prescaler - 1) | BT_SJW(0)
      | BT_TSEG1(CONFIG_PLATFORM_NXP_CAN_TSEG1 - 1)
      | BT_TSEG2(CONFIG_PLATFORM_NXP_CAN_TSEG2 - 1);

  return regValue;
}
/*----------------------------------------------------------------------------*/
static void changeMode(struct Can *interface, enum Mode mode)
{
  if (interface->mode != mode)
  {
    interface->mode = mode;

    LPC_CAN_Type * const reg = interface->base.reg;
    uint32_t test = 0;

    switch (interface->mode)
    {
      case MODE_LISTENER:
        test = TEST_SILENT;
        break;

      case MODE_LOOPBACK:
        test = TEST_SILENT | TEST_LBACK;
        break;

      default:
        break;
    }

    reg->CNTL |= CNTL_TEST;
    reg->TEST = test;
    reg->CNTL &= ~CNTL_TEST;
  }
}
/*----------------------------------------------------------------------------*/
static void changeRate(struct Can *interface, uint32_t rate)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  reg->CNTL |= CNTL_CCE;
  reg->BRPE = 0;
  reg->BT = calcBusTimings(interface, rate);
  reg->CNTL &= ~CNTL_CCE;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t id;
  bool event = false;

  while ((id = INT_ID_VALUE(reg->INT)) != 0)
  {
    if (id == INT_STATUS)
    {
      /* Clear all flags except TXOK and RXOK */
      reg->STAT &= STAT_TXOK | STAT_RXOK;
    }
    else if (STAT_LEC_VALUE(reg->STAT) == LEC_NO_ERROR)
    {
      reg->STAT &= ~(STAT_RXOK | STAT_TXOK); // TODO Move

      // TODO Rewrite and optimize buffer invalidation
      if (id >= TX_OBJECT)
      {
        invalidateMessage(interface, id);
      }
      else if (reg->ND1 & (1UL << (id - 1)))
      {
        if (!arrayEmpty(&interface->pool))
        {
          struct CanMessage *message;

          arrayPopBack(&interface->pool, &message);
          readMessage(interface, message, id);
          queuePush(&interface->rxQueue, &message);
          event = true;
        }

        /* Received message will be lost when queue is full */
        listenMessage(interface, id);
      }
    }
  }

  if (!queueEmpty(&interface->txQueue) && !reg->TXREQ2)
  {
    const size_t pendingMessages = queueSize(&interface->txQueue);
    const size_t lastMessageIndex = pendingMessages < MAX_MESSAGES / 2 ?
        pendingMessages : MAX_MESSAGES / 2;

    for (size_t index = 0; index <= lastMessageIndex; ++index)
    {
      const struct CanMessage *message;

      queuePop(&interface->txQueue, &message);
      sendMessage(interface, message, TX_OBJECT + index,
          index == lastMessageIndex);
      arrayPushBack(&interface->pool, &message);
    }
  }

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void invalidateMessage(struct Can *interface, unsigned int index)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Wait until previous read/write action is finished */
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);

  /* Clear Message Valid flag */
  reg->IF[0].ARB1 = 0;
  reg->IF[0].ARB2 = 0;

  /* Disable reception and transmission interrupts, clear transmit request */
  reg->IF[0].MCTRL = MCTRL_EOB | MCTRL_DLC(8);

  /* Write control bits back */
  reg->IF[0].CMDMSK = CMDMSK_WR | CMDMSK_CLRINTPND | CMDMSK_CTRL | CMDMSK_ARB;
  reg->IF[0].CMDREQ = index;
}
/*----------------------------------------------------------------------------*/
//TODO Make FIFO
static void listenMessage(struct Can *interface, unsigned int index)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Wait until previous read/write action is finished */
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);

  reg->IF[0].MCTRL = MCTRL_UMASK | MCTRL_RXIE | MCTRL_EOB | MCTRL_DLC(8);

  reg->IF[0].MSK1 = 0;
  reg->IF[0].MSK2 = 0;

  reg->IF[0].ARB1 = 0;
  reg->IF[0].ARB2 = ARB2_MSGVAL;

  reg->IF[0].CMDMSK = CMDMSK_CLRINTPND | CMDMSK_CTRL | CMDMSK_ARB | CMDMSK_MASK
      | CMDMSK_WR;
  reg->IF[0].CMDREQ = index;
}
/*----------------------------------------------------------------------------*/
static void readMessage(struct Can *interface, struct CanMessage *message,
    unsigned int index)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Read from Message Object to system memory */
  reg->IF[0].CMDMSK = CMDMSK_DATA_A | CMDMSK_DATA_B | CMDMSK_CTRL | CMDMSK_ARB;
  reg->IF[0].CMDREQ = index;
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);

  /* Fill message structure */
  const uint32_t arb1 = reg->IF[0].ARB1;
  const uint32_t arb2 = reg->IF[0].ARB2;
  const uint16_t data[] = {
      reg->IF[0].DA1,
      reg->IF[0].DA2,
      reg->IF[0].DB1,
      reg->IF[0].DB2
  };

  message->flags = 0;
  message->length = MCTRL_DLC_VALUE(reg->IF[0].MCTRL);
  memcpy(message->data, data, message->length);

  if (arb2 & ARB2_XTD)
  {
    message->id = EXT_ID_FROM_ARB(arb1, arb2);
    message->flags |= CAN_EXT_ID;
  }
  else
  {
    message->id = ARB2_STD_ID_VALUE(arb2);
  }

  if (arb2 & ARB2_DIR)
  {
    message->flags |= CAN_RTR;
  }
}
/*----------------------------------------------------------------------------*/
static void sendMessage(struct Can *interface, const struct CanMessage *message,
    unsigned int index, bool last)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Wait until previous read/write action is finished */
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);

  /* Configure message control register */
  uint32_t control = MCTRL_TXIE | MCTRL_TXRQST | MCTRL_DLC(message->length);

  if (last)
    control |= MCTRL_EOB;

  reg->IF[0].MCTRL = control;

  /* Configure command arbitration registers */
  uint32_t arb1 = 0;
  uint32_t arb2 = ARB2_MSGVAL;
  uint32_t mask = CMDMSK_CLRINTPND | CMDMSK_CTRL | CMDMSK_ARB | CMDMSK_WR;

  if (message->flags & CAN_EXT_ID)
  {
    /* Extended frame */
    assert(message->id < (1UL << 29));

    arb1 = ARB1_EXT_ID_FROM_ID(message->id);
    arb2 |= ARB2_EXT_ID_FROM_ID(message->id) | ARB2_XTD;
  }
  else
  {
    /* Standard frame */
    assert(message->id < (1UL << 11));

    arb2 |= ARB2_STD_ID_FROM_ID(message->id);
  }

  if (!(message->flags & CAN_RTR))
  {
    uint16_t data[4] = {0};

    /* Message is not a Remote Transmission Request */
    arb2 |= ARB2_DIR;

    /* Fill data registers */
    memcpy(data, message->data, message->length);

    reg->IF[0].DA1 = data[0];
    reg->IF[0].DA2 = data[1];
    reg->IF[0].DB1 = data[2];
    reg->IF[0].DB2 = data[3];

    /* Issue copying of data to Message Object */
    mask |= CMDMSK_DATA_A | CMDMSK_DATA_B;
  }

  reg->IF[0].ARB1 = arb1;
  reg->IF[0].ARB2 = arb2;

  reg->IF[0].CMDMSK = mask;
  reg->IF[0].CMDREQ = index;
}
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct CanConfig * const config = configBase;
  assert(config);

  const struct CanBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct Can * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = CanBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = 0;
  interface->timer = config->timer;
  interface->mode = MODE_LISTENER;

  const size_t poolSize = config->rxBuffers + config->txBuffers;

  res = arrayInit(&interface->pool, sizeof(struct CanMessage *), poolSize);
  if (res != E_OK)
    return res;
  res = queueInit(&interface->rxQueue, sizeof(struct CanMessage *),
      config->rxBuffers);
  if (res != E_OK)
    return res;
  res = queueInit(&interface->txQueue, sizeof(struct CanMessage *),
      config->txBuffers);
  if (res != E_OK)
    return res;

  interface->poolBuffer = malloc(sizeof(struct CanStandardMessage) * poolSize);

  struct CanStandardMessage *message = interface->poolBuffer;

  for (size_t index = 0; index < poolSize; ++index)
  {
    arrayPushBack(&interface->pool, &message);
    ++message;
  }

  LPC_CAN_Type * const reg = interface->base.reg;

  reg->CNTL = CNTL_INIT;

  /* Configure clock divider to achieve 50 MHz or less peripheral clock */
  const uint32_t frequency = canGetClock(&interface->base);
  const uint32_t divisor = (frequency + (MAX_FREQUENCY - 1)) / MAX_FREQUENCY;

  reg->CLKDIV = divisor - 1;

  /* Configure bit timing */
  interface->rate = config->rate;
  changeRate(interface, interface->rate);

  /* Message Objects should be reset manually */
  for (unsigned int index = 0; index < MAX_MESSAGES; ++index)
    invalidateMessage(interface, index);

  /* TODO Prepare Message Objects for reception */
  listenMessage(interface, RX_OBJECT);

  reg->CNTL = CNTL_IE | CNTL_SIE | CNTL_EIE;
  while (reg->CNTL & CNTL_INIT);

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void canDeinit(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);
  reg->CNTL = CNTL_INIT;

  queueDeinit(&interface->txQueue);
  queueDeinit(&interface->rxQueue);
  arrayDeinit(&interface->pool);

  CanBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum Result canSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Can * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result canGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Can * const interface = object;

  switch (parameter)
  {
    case IF_AVAILABLE:
      *(size_t *)data = queueSize(&interface->rxQueue);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = queueSize(&interface->txQueue);
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = interface->rate;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result canSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Can * const interface = object;

  switch ((enum CanParameter)parameter)
  {
    case IF_CAN_ACTIVE:
      changeMode(interface, MODE_ACTIVE);
      return E_OK;

    case IF_CAN_LISTENER:
      changeMode(interface, MODE_LISTENER);
      return E_OK;

    case IF_CAN_LOOPBACK:
      changeMode(interface, MODE_LOOPBACK);
      return E_OK;

    default:
      break;
  }

  switch (parameter)
  {
    case IF_RATE:
    {
      const uint32_t rate = *(const uint32_t *)data;

      changeRate(interface, rate);
      interface->rate = rate;

      return E_OK;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t canRead(void *object, void *buffer, size_t length)
{
  assert(length % sizeof(struct CanStandardMessage) == 0);

  struct Can * const interface = object;
  struct CanStandardMessage *output = buffer;
  size_t read = 0;

  while (read < length && !queueEmpty(&interface->rxQueue))
  {
    struct CanMessage *input;
    const IrqState state = irqSave();

    queuePop(&interface->rxQueue, &input);
    memcpy(output, input, sizeof(*output));
    arrayPushBack(&interface->pool, &input);

    irqRestore(state);

    read += sizeof(*output);
    ++output;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t canWrite(void *object, const void *buffer, size_t length)
{
  assert(length % sizeof(struct CanStandardMessage) == 0);

  struct Can * const interface = object;

  if (interface->mode == MODE_LISTENER)
    return 0;

  LPC_CAN_Type * const reg = interface->base.reg;
  const struct CanStandardMessage *input = buffer;
  const size_t initialLength = length;

  /* Synchronize access to the message queue and the CAN core */
  const IrqState state = irqSave();

  if (queueEmpty(&interface->txQueue) && !reg->TXREQ2)
  {
    const size_t totalMessages = length / sizeof(struct CanStandardMessage);
    const size_t messageCount = totalMessages < MAX_MESSAGES / 2 ?
        totalMessages : MAX_MESSAGES / 2;

    for (size_t index = 0; index < messageCount; ++index)
    {
      sendMessage(interface, (const struct CanMessage *)(input + index),
          TX_OBJECT + index, index == (messageCount - 1));
    }

    length -= messageCount * sizeof(struct CanStandardMessage);
    input += messageCount;
  }

  while (length && !queueFull(&interface->txQueue))
  {
    struct CanMessage *output;

    arrayPopBack(&interface->pool, &output);
    memcpy(output, input, sizeof(*input));
    queuePush(&interface->txQueue, &output);
    length -= sizeof(*input);
    ++input;
  }

  irqRestore(state);

  return initialLength - length;
}
