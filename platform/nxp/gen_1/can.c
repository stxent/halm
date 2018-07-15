/*
 * can.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcore/asm.h>
#include <halm/generic/can.h>
#include <halm/platform/nxp/gen_1/can.h>
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
static void readMessage(struct Can *, struct CanMessage *);
static void sendMessage(struct Can *, const struct CanMessage *, uint32_t *);

#ifdef CONFIG_PLATFORM_NXP_CAN_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);
static enum Result canSetCallback(void *, void (*)(void *), void *);
static enum Result canGetParam(void *, enum IfParameter, void *);
static enum Result canSetParam(void *, enum IfParameter, const void *);
static size_t canRead(void *, void *, size_t);
static size_t canWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_CAN_NO_DEINIT
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
      1 + CONFIG_PLATFORM_NXP_CAN_TSEG1 + CONFIG_PLATFORM_NXP_CAN_TSEG2;
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

      if (!queueFull(&interface->rxQueue))
      {
        struct CanMessage *message;

        arrayPopBack(&interface->pool, &message);
        readMessage(interface, message);
        message->timestamp = timestamp;
        queuePush(&interface->rxQueue, &message);
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
      while (!queueEmpty(&interface->txQueue) && status)
      {
        const struct CanMessage *message;

        queuePop(&interface->txQueue, &message);
        sendMessage(interface, message, &status);
        arrayPushBack(&interface->pool, &message);

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
static void readMessage(struct Can *interface, struct CanMessage *message)
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

    memcpy(message->data, data, message->length);
  }

  if (information & RFS_FF)
  {
    message->flags |= CAN_EXT_ID;
  }
}
/*----------------------------------------------------------------------------*/
static void sendMessage(struct Can *interface,
    const struct CanMessage *message, uint32_t *status)
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
    uint32_t data[2] = {0};

    memcpy(data, message->data, message->length);

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
  interface->sequence = 0;

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

  /* Activate Listen Only mode and enable local priority for transmit buffers */
  reg->MOD = MOD_LOM | MOD_TPM;

  LPC_CANAF->AFMR = AFMR_AccBP; //FIXME

#ifdef CONFIG_PLATFORM_NXP_CAN_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, config->priority);

  /* Enable interrupts on message reception and bus error */
  reg->IER = IER_RIE | IER_EPIE | IER_BEIE | IER_TIE_MASK;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_CAN_NO_DEINIT
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
#endif
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

  /* Acquire exclusive access to the message queue */
  irqDisable(interface->base.irq);

  if (queueEmpty(&interface->txQueue))
  {
    uint32_t status = reg->SR & SR_TBS_MASK;

    if (interface->sequence || status == SR_TBS_MASK)
    {
      while (length && status)
      {
        /* One of transmit buffers is empty, write new message into it */
        sendMessage(interface, (const struct CanMessage *)input, &status);

        length -= sizeof(*input);
        ++input;

        /* Stop after sequence restart */
        if (!interface->sequence)
          break;
      }
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

  irqEnable(interface->base.irq);

  return initialLength - length;
}
