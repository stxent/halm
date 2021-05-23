/*
 * can.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/can.h>
#include <halm/platform/lpc/can.h>
#include <halm/platform/lpc/gen_1/can_defs.h>
#include <halm/pm.h>
#include <halm/timer.h>
#include <xcore/accel.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
static void readMessage(struct Can *, struct CANMessage *);
static void resetQueues(struct Can *);
static void sendMessage(struct Can *, const struct CANMessage *, uint32_t *);

#ifdef CONFIG_PLATFORM_LPC_CAN_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);
static void canSetCallback(void *, void (*)(void *), void *);
static enum Result canGetParam(void *, int, void *);
static enum Result canSetParam(void *, int, const void *);
static size_t canRead(void *, void *, size_t);
static size_t canWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_CAN_NO_DEINIT
static void canDeinit(void *);
#else
#define canDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Can = &(const struct InterfaceClass){
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
static uint32_t calcBusTimings(const struct Can *interface, uint32_t rate)
{
  assert(rate != 0);

  const unsigned int bitsPerFrame =
      1 + CONFIG_PLATFORM_LPC_CAN_TSEG1 + CONFIG_PLATFORM_LPC_CAN_TSEG2;
  const uint32_t clock = canGetClock(&interface->base);
  const uint32_t prescaler = clock / rate / bitsPerFrame;
  const uint32_t regValue = BTR_BRP(prescaler - 1) | BTR_SJW(0)
      | BTR_TSEG1(CONFIG_PLATFORM_LPC_CAN_TSEG1 - 1)
      | BTR_TSEG2(CONFIG_PLATFORM_LPC_CAN_TSEG2 - 1);

  return regValue;
}
/*----------------------------------------------------------------------------*/
static void changeMode(struct Can *interface, enum Mode mode)
{
  if (interface->mode != mode)
  {
    interface->mode = mode;

    LPC_CAN_Type * const reg = interface->base.reg;
    uint32_t value = reg->MOD & ~(MOD_LOM | MOD_STM);

    switch (interface->mode)
    {
      case MODE_LISTENER:
        /*
         * Notice: CAN peripheral in the Listen Only mode cannot receive
         * unacknowledged messages due to a hardware issue in the CPU.
         */
        value |= MOD_LOM;
        break;

      case MODE_LOOPBACK:
        value |= MOD_STM;
        break;

      default:
        break;
    }

    /* Return pending message descriptors to the pool */
    resetQueues(interface);

    /* Enable Reset mode to configure LOM and STM bits */
    reg->MOD |= MOD_RM;
    /* Change test settings */
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
  const uint32_t icr = reg->ICR;
  bool event = false;

  if (icr & ICR_RI)
  {
    while (reg->SR & SR_RBS)
    {
      const uint32_t timestamp = interface->timer ?
          timerGetValue(interface->timer) : 0;

      if (!pointerQueueFull(&interface->rxQueue))
      {
        struct CANMessage * const message = pointerArrayBack(&interface->pool);
        pointerArrayPopBack(&interface->pool);

        readMessage(interface, message);
        message->timestamp = timestamp;
        pointerQueuePushBack(&interface->rxQueue, message);
        event = true;
      }

      /* Release receive buffer */
      reg->CMR = CMR_RRB | CMR_CDO;
    }
  }

  const uint32_t sr = reg->SR;

  if ((icr & (ICR_EPI | ICR_TI_MASK)) && !(sr & SR_BS))
  {
    uint32_t status = sr & SR_TBS_MASK;

    /*
     * Enqueue new messages when:
     *   - no sequence restart occurred.
     *   - when sequence restart occurred and hardware queue was drained.
     */
    if (interface->sequence || status == SR_TBS_MASK)
    {
      while (!pointerQueueEmpty(&interface->txQueue) && status)
      {
        struct CANMessage * const message =
            pointerQueueFront(&interface->txQueue);
        pointerQueuePopFront(&interface->txQueue);

        sendMessage(interface, message, &status);
        pointerArrayPushBack(&interface->pool, message);

        /* Check whether sequence restart occurred or not */
        if (!interface->sequence)
          break;
      }
    }
  }

  if ((icr & ICR_BEI) && (sr & SR_BS))
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
static void readMessage(struct Can *interface, struct CANMessage *message)
{
  LPC_CAN_Type * const reg = interface->base.reg;
  const uint32_t information = reg->RFS;

  message->id = reg->RID;
  message->flags = 0;
  message->length = RFS_DLC_VALUE(information);

  if (information & RFS_RTR)
  {
    message->flags |= CAN_RTR;
  }
  else
  {
    const uint32_t data[2] = {
        reg->RDA,
        reg->RDB
    };

    memcpy(message->data, data, sizeof(data));
  }

  if (information & RFS_FF)
  {
    message->flags |= CAN_EXT_ID;
  }
}
/*----------------------------------------------------------------------------*/
static void resetQueues(struct Can *interface)
{
  irqDisable(interface->base.irq);

  while (!pointerQueueEmpty(&interface->txQueue))
  {
    struct CANMessage * const message = pointerQueueFront(&interface->txQueue);
    pointerQueuePopFront(&interface->txQueue);
    pointerArrayPushBack(&interface->pool, message);
  }

  while (!pointerQueueEmpty(&interface->rxQueue))
  {
    struct CANMessage * const message = pointerQueueFront(&interface->rxQueue);
    pointerQueuePopFront(&interface->rxQueue);
    pointerArrayPushBack(&interface->pool, message);
  }

  irqEnable(interface->base.irq);
}
/*----------------------------------------------------------------------------*/
static void sendMessage(struct Can *interface,
    const struct CANMessage *message, uint32_t *status)
{
  assert(message->length <= 8);

  uint32_t command = interface->mode != MODE_LOOPBACK ? CMR_TR : CMR_SRR;
  uint32_t information = TFI_DLC(message->length);

  if (message->flags & CAN_EXT_ID)
  {
    assert(message->id < (1UL << 29));
    information |= TFI_FF;
  }
  else
  {
    assert(message->id < (1UL << 11));
  }

  LPC_CAN_Type * const reg = interface->base.reg;
  const unsigned int position = countLeadingZeros32(reverseBits32(*status));
  const unsigned int index = SR_TBS_VALUE_TO_CHANNEL(position);

  if (!(message->flags & CAN_RTR))
  {
    uint32_t data[2];
    memcpy(data, message->data, sizeof(data));

    reg->TX[index].TDA = data[0];
    reg->TX[index].TDB = data[1];
  }
  else
  {
    information |= TFI_RTR;
  }

  reg->TX[index].TFI = information | TFI_PRIO(interface->sequence);
  reg->TX[index].TID = message->id;

  reg->CMR = command | CMR_STB(index);

  *status &= ~BIT(position);
  ++interface->sequence;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_CAN_PM
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
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct Can * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = CanBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->callback = 0;
  interface->timer = config->timer;
  interface->mode = MODE_LISTENER;
  interface->sequence = 0;

  const size_t poolSize = config->rxBuffers + config->txBuffers;

  if (!pointerArrayInit(&interface->pool, poolSize))
    return E_MEMORY;
  if (!pointerQueueInit(&interface->rxQueue, config->rxBuffers))
    return E_MEMORY;
  if (!pointerQueueInit(&interface->txQueue, config->txBuffers))
    return E_MEMORY;

  interface->poolBuffer = malloc(sizeof(struct CANStandardMessage) * poolSize);

  struct CANStandardMessage *message = interface->poolBuffer;

  for (size_t index = 0; index < poolSize; ++index)
  {
    pointerArrayPushBack(&interface->pool, message);
    ++message;
  }

  LPC_CAN_Type * const reg = interface->base.reg;

  reg->MOD = MOD_RM; /* Reset CAN */
  reg->IER = 0; /* Disable Receive Interrupt */
  reg->GSR = 0; /* Reset error counter */

  interface->rate = config->rate;
  reg->BTR = calcBusTimings(interface, interface->rate);

  /* Activate Listen Only mode and enable local priority for transmit buffers */
  reg->MOD = MOD_LOM | MOD_TPM;

  LPC_CANAF->AFMR = AFMR_AccBP; //FIXME

#ifdef CONFIG_PLATFORM_LPC_CAN_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable interrupts on message reception and bus error */
  reg->IER = IER_RIE | IER_EPIE | IER_BEIE | IER_TIE_MASK;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_CAN_NO_DEINIT
static void canDeinit(void *object)
{
  struct Can * const interface = object;
  LPC_CAN_Type * const reg = interface->base.reg;

  /* Disable all interrupts */
  irqDisable(interface->base.irq);
  reg->IER = 0;

#ifdef CONFIG_PLATFORM_LPC_CAN_PM
  pmUnregister(interface);
#endif

  pointerQueueDeinit(&interface->txQueue);
  pointerQueueDeinit(&interface->rxQueue);
  pointerArrayDeinit(&interface->pool);
  CanBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void canSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Can * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result canGetParam(void *object, int parameter, void *data)
{
  struct Can * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_AVAILABLE:
      *(size_t *)data = pointerQueueSize(&interface->rxQueue)
          * sizeof(struct CANStandardMessage);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = pointerQueueSize(&interface->txQueue)
          * sizeof(struct CANStandardMessage);
      return E_OK;

    case IF_RATE:
      *(uint32_t *)data = interface->rate;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result canSetParam(void *object, int parameter, const void *data)
{
  struct Can * const interface = object;

  switch ((enum CANParameter)parameter)
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

  switch ((enum IfParameter)parameter)
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
  assert(length % sizeof(struct CANStandardMessage) == 0);

  struct Can * const interface = object;
  struct CANStandardMessage *current = buffer;
  const struct CANStandardMessage * const last =
      (const void *)((uintptr_t)buffer + length);

  while (!pointerQueueEmpty(&interface->rxQueue) && current < last)
  {
    struct CANMessage * const input = pointerQueueFront(&interface->rxQueue);
    memcpy(current, input, sizeof(*current));

    irqDisable(interface->base.irq);
    pointerQueuePopFront(&interface->rxQueue);
    pointerArrayPushBack(&interface->pool, input);
    irqEnable(interface->base.irq);

    ++current;
  }

  return (uintptr_t)current - (uintptr_t)buffer;
}
/*----------------------------------------------------------------------------*/
static size_t canWrite(void *object, const void *buffer, size_t length)
{
  assert(length % sizeof(struct CANStandardMessage) == 0);

  struct Can * const interface = object;
  const struct CANStandardMessage *current = buffer;
  const struct CANStandardMessage * const last =
      (const void *)((uintptr_t)buffer + length);

  /* Acquire exclusive access to the message queue */
  irqDisable(interface->base.irq);

  if (pointerQueueEmpty(&interface->txQueue))
  {
    LPC_CAN_Type * const reg = interface->base.reg;
    uint32_t status = reg->SR & SR_TBS_MASK;

    if (interface->sequence || status == SR_TBS_MASK)
    {
      while (current < last && status != 0)
      {
        /* One of transmit buffers is empty, write new message into it */
        sendMessage(interface, (const struct CANMessage *)current, &status);
        ++current;

        /* Stop after sequence restart */
        if (!interface->sequence)
          break;
      }
    }
  }

  while (current < last && !pointerQueueFull(&interface->txQueue))
  {
    struct CANMessage * const output = pointerArrayBack(&interface->pool);
    pointerArrayPopBack(&interface->pool);

    memcpy(output, current, sizeof(*current));
    pointerQueuePushBack(&interface->txQueue, output);

    ++current;
  }

  irqEnable(interface->base.irq);
  return (uintptr_t)current - (uintptr_t)buffer;
}
