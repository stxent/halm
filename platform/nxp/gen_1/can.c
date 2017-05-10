/*
 * can.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <halm/generic/can.h>
#include <halm/platform/nxp/can.h>
#include <halm/platform/nxp/gen_1/can_defs.h>
#include <halm/pm.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
enum Mode
{
  MODE_LISTENER,
  MODE_ACTIVE,
  MODE_LOOPBACK
};
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(const struct Can *, uint32_t);
static void changeMode(struct Can *, enum Mode);
static void changeRate(struct Can *, uint32_t);
static void interruptHandler(void *);
static uint32_t sendMessage(struct Can *, const struct CanMessage *, uint32_t);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_CAN_PM
static void powerStateHandler(void *, enum PmState);
#endif
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

  const unsigned int bitsPerFrame = 1 + CONFIG_PLATFORM_NXP_CAN_TSEG1
      + CONFIG_PLATFORM_NXP_CAN_TSEG2;
  const uint32_t clock = canGetClock(&interface->base);
  const uint32_t prescaler = clock / rate / bitsPerFrame;
  const uint32_t regValue = BTR_BRP(prescaler - 1) | BTR_SJW(0)
      | BTR_TSEG1(CONFIG_PLATFORM_NXP_CAN_TSEG1 - 1)
      | BTR_TSEG2(CONFIG_PLATFORM_NXP_CAN_TSEG2 - 1);

  return regValue;
}
/*----------------------------------------------------------------------------*/
static void changeMode(struct Can *interface, enum Mode mode)
{
  if (interface->mode != mode)
  {
    interface->mode = mode;

    LPC_CAN_Type * const reg = interface->base.reg;
    uint32_t value = reg->MOD;

    switch (interface->mode)
    {
      case MODE_LISTENER:
        value = (value & ~MOD_STM) | MOD_LOM;
        break;

      case MODE_ACTIVE:
        value = value & ~(MOD_LOM | MOD_STM);
        break;

      case MODE_LOOPBACK:
        value = (value & ~MOD_LOM) | MOD_STM;
        break;
    }

    /* LOM and STM bits can be written only in the Reset mode */
    reg->MOD |= MOD_RM;
    /* Error counters can be cleared only in the Reset mode */
    reg->GSR &= ~(GSR_RXERR_MASK | GSR_TXERR_MASK);

    reg->MOD = value;
  }
}
/*----------------------------------------------------------------------------*/
static void changeRate(struct Can *interface, uint32_t rate)
{
  LPC_CAN_Type * const reg = interface->base.reg;

  reg->MOD |= MOD_RM; /* Enable Reset mode */
  reg->BTR = calcBusTimings(interface, rate);
  reg->MOD &= ~MOD_RM;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;
  bool event = false;

  while (reg->SR & SR_RBS)
  {
    if (!arrayEmpty(&interface->pool))
    {
      const uint32_t timestamp = interface->timer ?
          timerGetValue(interface->timer) : 0;

      const uint32_t data[2] = {reg->RDA, reg->RDB};
      const uint32_t information = reg->RFS;
      struct CanStandardMessage *message;

      arrayPopBack(&interface->pool, &message);

      message->timestamp = timestamp;
      message->id = reg->RID;
      message->length = RFS_DLC_VALUE(information);
      message->flags = 0;
      if (information & RFS_FF)
        message->flags |= CAN_EXT_ID;
      if (information & RFS_RTR)
        message->flags |= CAN_RTR;
      memcpy(message->data, data, sizeof(data));

      queuePush(&interface->rxQueue, &message);
      event = true;
    }

    /* Release receive buffer */
    reg->CMR = CMR_RRB;
  }

  uint32_t status = reg->SR;

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
      arrayPushBack(&interface->pool, &message);
    }
  }

  if (status & SR_BS)
  {
    /*
     * The controller is forced into a bus-off state, RM bit should be cleared
     * to continue normal operation.
     */
    reg->MOD &= ~MOD_RM;
  }

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static uint32_t sendMessage(struct Can *interface,
    const struct CanMessage *message, uint32_t status)
{
  assert(message->length <= 8);

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

  uint32_t command = CMR_TR | CMR_STB(index);
  uint32_t information = TFI_DLC(message->length);

  if ((message->flags & CAN_SELF_RX) || interface->mode == MODE_LOOPBACK)
    command |= CMR_SRR;

  if (message->flags & CAN_EXT_ID)
  {
    assert(message->id < (1UL << 29));
    information |= TFI_FF;
  }
  else
  {
    assert(message->id < (1UL << 11));
  }

  if (!(message->flags & CAN_RTR))
  {
    uint32_t data[2] = {0};

    memcpy(data, message->data, message->length);

    reg->TX[index].TDA = data[0];
    reg->TX[index].TDB = data[1];
  }
  else
  {
    information |= TFI_RTR;
  }

  reg->TX[index].TFI = information;
  reg->TX[index].TID = message->id;

  reg->IER |= mask;
  reg->CMR = command;

  return status & ~SR_TBS(index);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_CAN_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct Can * const interface = object;

    changeRate(interface, interface->rate);
  }
}
#endif
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

  reg->MOD = MOD_RM; /* Reset CAN */
  reg->IER = 0; /* Disable Receive Interrupt */
  reg->GSR = 0; /* Reset error counter */

  interface->rate = config->rate;
  reg->BTR = calcBusTimings(interface, interface->rate);

  /* Disable Reset mode and activate Listen Only mode */
  reg->MOD = MOD_LOM;

  LPC_CANAF->AFMR = AFMR_AccBP; //FIXME

#ifdef CONFIG_PLATFORM_NXP_CAN_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, config->priority);

  /* Enable interrupts on message reception and bus error */
  reg->IER = IER_RIE | IER_BEIE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void canDeinit(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Disable all interrupts */
  reg->IER = 0;

#ifdef CONFIG_PLATFORM_NXP_CAN_PM
  pmUnregister(interface);
#endif

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

    arrayPopBack(&interface->pool, &output);
    memcpy(output, input, sizeof(*input));
    queuePush(&interface->txQueue, &output);
    length -= sizeof(*input);
    ++input;
  }

  irqRestore(state);

  return initialLength - length;
}
