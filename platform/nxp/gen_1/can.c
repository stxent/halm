/*
 * can.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <halm/common/can.h>
#include <halm/platform/nxp/can.h>
#include <halm/platform/nxp/gen_1/can_defs.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
//TODO Move to kconfig
#define CONFIG_CAN_TSEG1 11
#define CONFIG_CAN_TSEG2 3
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(struct Can *, uint32_t, uint8_t, uint8_t);
static void interruptHandler(void *);
static uint32_t sendMessage(struct Can *, const struct CanMessage *, uint32_t);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_CAN_PM
static enum result powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result canInit(void *, const void *);
static void canDeinit(void *);
static enum result canCallback(void *, void (*)(void *), void *);
static enum result canGet(void *, enum ifOption, void *);
static enum result canSet(void *, enum ifOption, const void *);
static size_t canRead(void *, void *, size_t);
static size_t canWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass canTable = {
    .size = sizeof(struct Can),
    .init = canInit,
    .deinit = canDeinit,

    .callback = canCallback,
    .get = canGet,
    .set = canSet,
    .read = canRead,
    .write = canWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Can = &canTable;
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(struct Can *interface, uint32_t rate,
    uint8_t tseg1, uint8_t tseg2)
{
  assert(rate != 0);

  const unsigned int bitsPerFrame = 1 + tseg1 + tseg2;
  const uint32_t clock = canGetClock((struct CanBase *)interface);
  const uint32_t prescaler = clock / rate / bitsPerFrame;
  const uint32_t regValue = BTR_BRP(prescaler - 1) | BTR_SJW(0)
      | BTR_TSEG1(tseg1 - 1) | BTR_TSEG2(tseg2 - 1);

  return regValue;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t status = reg->SR;
  bool event = false;

  if (status & SR_RBS)
  {
    if (!queueEmpty(&interface->pool))
    {
      const uint32_t data[2] = {reg->RDA, reg->RDB};
      const uint32_t information = reg->RFS;
      struct CanStandardMessage *message;

      queuePop(&interface->pool, &message);

      message->id = reg->RID;
      message->length = RFS_DLC_VALUE(information);
      message->flags = 0;
      if (information & RFS_FF)
        message->flags |= CAN_EXTID;
      if (information & RFS_RTR)
        message->flags |= CAN_RTR;
      memcpy(message->data, data, sizeof(data));

      queuePush(&interface->rxQueue, &message);
      event = true;
    }

    /* Release receive buffer */
    reg->CMR = CMR_RRB;
  }

  if (status & SR_TBS_MASK)
  {
    /* Disable interrupts for completed transmit buffers */
    uint32_t enabledInterrupts = reg->IER;

    if (status & SR_TBS(0))
      enabledInterrupts &= ~IER_TIE1;
    if (status & SR_TBS(1))
      enabledInterrupts &= ~IER_TIE2;
    if (status & SR_TBS(2))
      enabledInterrupts &= ~IER_TIE3;

    reg->IER = enabledInterrupts;

    while (!queueEmpty(&interface->txQueue) && (status & SR_TBS_MASK))
    {
      const struct CanMessage *message;

      queuePop(&interface->txQueue, &message);
      status = sendMessage(interface, message, status);
    }
  }

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static uint32_t sendMessage(struct Can *interface,
    const struct CanMessage *message, uint32_t status)
{
  LPC_CAN_Type * const reg = interface->base.reg;
  unsigned int index;
  uint32_t mask;

  if (status & SR_TBS(0))
  {
    index = 0;
    mask = IER_TIE1;
  }
  else if (status & SR_TBS(1))
  {
    index = 1;
    mask = IER_TIE2;
  }
  else
  {
    index = 2;
    mask = IER_TIE3;
  }

  //TODO Assert length, id

  uint32_t data[2] = {0};
  uint32_t information = TFI_DLC(message->length);

  if (message->flags & CAN_EXTID)
    information |= TFI_FF;
  if (message->flags & CAN_RTR)
    information |= TFI_RTR;

  memcpy(data, message->data, message->length);

  reg->TX[index].TFI = information;
  reg->TX[index].TID = message->id;
  reg->TX[index].TDA = data[0];
  reg->TX[index].TDB = data[1];

  reg->CMR = CMR_TR | CMR_STB(index);
  reg->IER |= mask;

  return status & ~SR_TBS(index);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_CAN_PM
static enum result powerStateHandler(void *object, enum pmState state)
{
  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
static enum result canInit(void *object, const void *configBase)
{
  const struct CanConfig * const config = configBase;
  const struct CanBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct Can * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = CanBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = 0;
  interface->rate = config->rate;

  //FIXME
  const size_t poolSize = config->rxBuffers + config->txBuffers;

  res = queueInit(&interface->pool, 4, poolSize);
  if (res != E_OK)
    return res;
  res = queueInit(&interface->rxQueue, 4, config->rxBuffers);
  if (res != E_OK)
    return res;
  res = queueInit(&interface->txQueue, 4, config->txBuffers);
  if (res != E_OK)
    return res;

  //TODO CAN FD
  interface->poolBuffer = malloc(sizeof(struct CanStandardMessage) * poolSize);

  struct CanStandardMessage *message = interface->poolBuffer;

  for (size_t index = 0; index < poolSize; ++index)
  {
    queuePush(&interface->pool, &message);
    ++message;
  }

  LPC_CAN_Type * const reg = interface->base.reg;

  reg->MOD = MOD_RM; /* Reset CAN */
  reg->IER = 0; /* Disable Receive Interrupt */
  reg->GSR = 0; /* Reset error counter */

  reg->BTR = calcBusTimings(interface, config->rate, CONFIG_CAN_TSEG1,
      CONFIG_CAN_TSEG2);
  reg->MOD = 0; /* CAN in normal operation mode */

  LPC_CANAF->AFMR = AFMR_AccBP; //FIXME

#ifdef CONFIG_CAN_PM
  if ((res = pmRegister(interface, powerStateHandler)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, config->priority);

  reg->IER = IER_RIE; /* Enable receive interrupts */

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void canDeinit(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Disable all interrupts */
  reg->IER = 0;

#ifdef CONFIG_CAN_PM
  pmUnregister(interface);
#endif

  queueDeinit(&interface->txQueue);
  queueDeinit(&interface->rxQueue);
  CanBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result canCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Can * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result canGet(void *object, enum ifOption option, void *data)
{
  struct Can * const interface = object;

  switch (option)
  {
//    case IF_AVAILABLE:
//      *(size_t *)data = byteQueueSize(&interface->rxQueue);
//      return E_OK;
//
//    case IF_PENDING:
//      *(size_t *)data = byteQueueSize(&interface->txQueue);
//      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum result canSet(void *object, enum ifOption option,
    const void *data)
{
  struct Can * const interface = object;

  switch (option)
  {
    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t canRead(void *object, void *buffer, size_t length)
{
  struct Can * const interface = object;
  size_t read = 0;

  assert(length % sizeof(struct CanStandardMessage) == 0);

  struct CanStandardMessage *output = buffer;

  while (length && !queueEmpty(&interface->rxQueue))
  {
    struct CanMessage *input;
    const irqState state = irqSave();

    queuePop(&interface->rxQueue, &input);
    memcpy(output, input, sizeof(*output));
    queuePush(&interface->pool, &input);

    irqRestore(state);

    read += sizeof(*output);
    ++output;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t canWrite(void *object, const void *buffer, size_t length)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;
  const size_t initialLength = length;

  assert(length % sizeof(struct CanStandardMessage) == 0);

  const struct CanStandardMessage *input = buffer;
  const irqState state = irqSave();

  if (queueEmpty(&interface->txQueue))
  {
    uint32_t status = reg->SR;

    while (length && (status & SR_TBS_MASK))
    {
      /* One of transmit buffers is empty */
      status = sendMessage(interface, (const struct CanMessage *)input, status);

      length -= sizeof(*input);
      ++input;
    }
  }

  while (length && !queueFull(&interface->txQueue))
  {
    struct CanMessage *output;

    queuePop(&interface->pool, &output);
    memcpy(output, input, sizeof(*input));
    queuePush(&interface->txQueue, &output);
    length -= sizeof(*input);
    ++input;
  }

  irqRestore(state);

  return initialLength - length;
}
