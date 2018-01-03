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
#include <halm/platform/nxp/gen_2/can.h>
#include <halm/platform/nxp/gen_2/can_defs.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY 50000000
#define MAX_MESSAGES  32

#define RX_OBJECT     1
#define RX_REG_INDEX  ((RX_OBJECT - 1) / 16)
#define TX_OBJECT     (1 + (MAX_MESSAGES / 2))
#define TX_REG_INDEX  ((TX_OBJECT - 1) / 16)
/*----------------------------------------------------------------------------*/
enum Mode
{
  MODE_LISTENER,
  MODE_ACTIVE,
  MODE_LOOPBACK
};
/*----------------------------------------------------------------------------*/
static void buildAcceptanceFilters(struct Can *);
static uint32_t calcBusTimings(const struct Can *, uint32_t);
static void changeMode(struct Can *, enum Mode);
static void changeRate(struct Can *, uint32_t);
static bool dropMessage(struct Can *, size_t);
static void interruptHandler(void *);
static void invalidateMessage(struct Can *, size_t);
static void listenForMessage(struct Can *, size_t, bool);
static bool readMessage(struct Can *, struct CanMessage *, size_t);
static void writeMessage(struct Can *, const struct CanMessage *, size_t, bool);
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
static void buildAcceptanceFilters(struct Can *interface)
{
  for (size_t index = RX_OBJECT; index < TX_OBJECT; ++index)
  {
    listenForMessage(interface, index, index == TX_OBJECT - 1);
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(const struct Can *interface, uint32_t rate)
{
  assert(rate != 0);

  const LPC_CAN_Type * const reg = interface->base.reg;

  const unsigned int bitsPerFrame =
      1 + CONFIG_PLATFORM_NXP_CAN_TSEG1 + CONFIG_PLATFORM_NXP_CAN_TSEG2;
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
  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t control = reg->CNTL | CNTL_TEST;
  uint32_t test = 0;

  /* Enable write access to the Test register */
  reg->CNTL = control;

  switch (mode)
  {
    case MODE_LISTENER:
      test = TEST_SILENT;
      break;

    case MODE_LOOPBACK:
      test = TEST_SILENT | TEST_LBACK;
      break;

    default:
      control &= ~CNTL_TEST;
      break;
  }

  /*
   * Test functions are only active when the TEST bit
   * in the Control register is set.
   */
  reg->TEST = test;
  reg->CNTL = control;
}
/*----------------------------------------------------------------------------*/
static void changeRate(struct Can *interface, uint32_t rate)
{
  LPC_CAN_Type * const reg = interface->base.reg;
  const uint32_t state = reg->CNTL;

  reg->CNTL = state | (CNTL_INIT | CNTL_CCE);
  reg->BRPE = 0;
  reg->BT = calcBusTimings(interface, rate);
  reg->CNTL = state;
}
/*----------------------------------------------------------------------------*/
static bool dropMessage(struct Can *interface, size_t index)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Clear pending interrupt and new data flag  */
  reg->IF[0].CMDMSK = CMDMSK_NEWDAT | CMDMSK_CLRINTPND | CMDMSK_CTRL;
  reg->IF[0].CMDREQ = index;
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);

  /* Return true when this Message Object is the end of the FIFO */
  return (reg->IF[0].MCTRL & MCTRL_EOB) != 0;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t id;
  bool event = false;

  while ((id = INT_ID_VALUE(reg->INT)) != INT_NONE)
  {
    const uint32_t status = reg->STAT;

    /* Clear all flags, update Last Error Code */
    reg->STAT = STAT_LEC(LEC_UNUSED);

    if (id == INT_STATUS)
    {
      if (status & STAT_BOFF)
      {
        /* Exit bus-off state */
        reg->CNTL &= ~CNTL_INIT;
      }
    }
    else if (id < TX_OBJECT)
    {
      /* Receive messages */
      bool endOfChain = false;

      while (!endOfChain && reg->ND[RX_REG_INDEX] & (1UL << (id - 1)))
      {
        const uint32_t timestamp = interface->timer ?
            timerGetValue(interface->timer) : 0;

        if (!queueFull(&interface->rxQueue))
        {
          struct CanMessage *message;

          arrayPopBack(&interface->pool, &message);
          endOfChain = readMessage(interface, message, id);
          message->timestamp = timestamp;
          queuePush(&interface->rxQueue, &message);
          event = true;
        }
        else
        {
          /* Received message will be lost when queue is full */
          endOfChain = dropMessage(interface, id);
        }

        ++id;
      }
    }
    else
    {
      /* Clear pending transmit interrupt */
      invalidateMessage(interface, id);
    }
  }

  if (!queueEmpty(&interface->txQueue) && !reg->TXREQ[TX_REG_INDEX])
  {
    const size_t pendingMessages = queueSize(&interface->txQueue);
    const size_t lastMessageIndex = pendingMessages >= MAX_MESSAGES / 2 ?
        (MAX_MESSAGES / 2 - 1) : pendingMessages;

    for (size_t index = 0; index <= lastMessageIndex; ++index)
    {
      const struct CanMessage *message;

      queuePop(&interface->txQueue, &message);
      writeMessage(interface, message, TX_OBJECT + index,
          index == lastMessageIndex);
      arrayPushBack(&interface->pool, &message);
    }
  }

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void invalidateMessage(struct Can *interface, size_t index)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Clear Message Valid flag */
  reg->IF[0].ARB1 = 0;
  reg->IF[0].ARB2 = 0;

  /* Disable reception and transmission interrupts, clear transmit request */
  reg->IF[0].MCTRL = 0;

  reg->IF[0].CMDMSK = CMDMSK_WR | CMDMSK_CTRL | CMDMSK_ARB;
  reg->IF[0].CMDREQ = index;

  /* Wait until read/write action is finished */
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);
}
/*----------------------------------------------------------------------------*/
static void listenForMessage(struct Can *interface, size_t index, bool last)
{
  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t control = MCTRL_UMASK | MCTRL_RXIE | MCTRL_DLC(8);

  if (last)
    control |= MCTRL_EOB;

  reg->IF[0].MCTRL = control;

  reg->IF[0].MSK1 = 0;
  reg->IF[0].MSK2 = 0;

  reg->IF[0].ARB1 = 0;
  reg->IF[0].ARB2 = ARB2_MSGVAL;

  reg->IF[0].CMDMSK = CMDMSK_WR | CMDMSK_CTRL | CMDMSK_ARB | CMDMSK_MASK;
  reg->IF[0].CMDREQ = index;

  /* Wait until read/write action is finished */
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);
}
/*----------------------------------------------------------------------------*/
static bool readMessage(struct Can *interface, struct CanMessage *message,
    size_t index)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Read from Message Object to system memory and clear interrupt flag */
  reg->IF[0].CMDMSK = CMDMSK_DATA_A | CMDMSK_DATA_B
      | CMDMSK_NEWDAT | CMDMSK_CLRINTPND | CMDMSK_CTRL | CMDMSK_ARB;
  reg->IF[0].CMDREQ = index;
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);

  const uint32_t arb1 = reg->IF[0].ARB1;
  const uint32_t arb2 = reg->IF[0].ARB2;
  const uint32_t control = reg->IF[0].MCTRL;

  /* Fill message structure */
  message->flags = 0;
  message->length = MCTRL_DLC_VALUE(control);

  if (arb2 & ARB2_DIR)
  {
    message->flags |= CAN_RTR;
  }
  else
  {
    const uint16_t data[] = {
        reg->IF[0].DA1,
        reg->IF[0].DA2,
        reg->IF[0].DB1,
        reg->IF[0].DB2
    };

    memcpy(message->data, data, message->length);
  }

  if (arb2 & ARB2_XTD)
  {
    message->id = EXT_ID_FROM_ARB(arb1, arb2);
    message->flags |= CAN_EXT_ID;
  }
  else
  {
    message->id = ARB2_STD_ID_VALUE(arb2);
  }

  /* Return true when this Message Object is the end of the FIFO */
  return (control & MCTRL_EOB) != 0;
}
/*----------------------------------------------------------------------------*/
static void writeMessage(struct Can *interface,
    const struct CanMessage *message, size_t index, bool last)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Prepare values for the message interface registers */
  uint32_t arb1 = 0;
  uint32_t arb2 = ARB2_MSGVAL;
  uint32_t control = MCTRL_EOB | MCTRL_TXRQST | MCTRL_NEWDAT;
  uint32_t mask = CMDMSK_WR | CMDMSK_CTRL | CMDMSK_ARB;

  if (last)
    control |= MCTRL_TXIE;

  if (!(message->flags & CAN_RTR))
  {
    uint16_t data[4] = {0};

    /* Message is not a Remote Transmission Request */
    arb2 |= ARB2_DIR;

    /* Fill data registers */
    control |= MCTRL_DLC(message->length);
    memcpy(data, message->data, message->length);

    reg->IF[0].DA1 = data[0];
    reg->IF[0].DA2 = data[1];
    reg->IF[0].DB1 = data[2];
    reg->IF[0].DB2 = data[3];

    /* Issue copying of data to Message Object */
    mask |= CMDMSK_DATA_A | CMDMSK_DATA_B;
  }

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

  reg->IF[0].MCTRL = control;

  reg->IF[0].ARB1 = arb1;
  reg->IF[0].ARB2 = arb2;

  reg->IF[0].CMDMSK = mask;
  reg->IF[0].CMDREQ = index;

  /* Wait until read/write action is finished */
  while (reg->IF[0].CMDREQ & CMDREQ_BUSY);
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

  /* Enable write access to the Test register and enter initialization mode */
  reg->CNTL = CNTL_INIT | CNTL_TEST;
  /* Enable listen only mode */
  reg->TEST = TEST_SILENT;

  /* Configure clock divider to achieve 50 MHz or less peripheral clock */
  const uint32_t frequency = canGetClock(&interface->base);
  const uint32_t divisor = (frequency + (MAX_FREQUENCY - 1)) / MAX_FREQUENCY;

  reg->CLKDIV = divisor - 1;

  /* Configure bit timing */
  interface->rate = config->rate;
  changeRate(interface, interface->rate);

  /* All Message Objects should be reset manually */
  for (size_t index = 1; index <= MAX_MESSAGES; ++index)
    invalidateMessage(interface, index);

  /* Prepare RX Message Objects */
  buildAcceptanceFilters(interface);

  /* Enable interrupts and exit initialization mode */
  reg->CNTL = (reg->CNTL & ~CNTL_INIT) | (CNTL_IE | CNTL_EIE);

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

    irqDisable(interface->base.irq);
    queuePop(&interface->rxQueue, &input);
    memcpy(output, input, sizeof(*output));
    arrayPushBack(&interface->pool, &input);
    irqEnable(interface->base.irq);

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
  LPC_CAN_Type * const reg = interface->base.reg;
  const struct CanStandardMessage *input = buffer;
  const size_t initialLength = length;

  /* Synchronize access to the message queue and the CAN core */
  irqDisable(interface->base.irq);

  if (queueEmpty(&interface->txQueue) && !reg->TXREQ[TX_REG_INDEX])
  {
    const size_t totalMessages = length / sizeof(struct CanStandardMessage);
    const size_t messageCount = MIN(totalMessages, MAX_MESSAGES / 2);

    for (size_t index = 0; index < messageCount; ++index)
    {
      writeMessage(interface, (const struct CanMessage *)(input + index),
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

  irqEnable(interface->base.irq);

  return initialLength - length;
}
