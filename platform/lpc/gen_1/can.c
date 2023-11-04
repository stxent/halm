/*
 * can.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/can.h>
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/lpc/can.h>
#include <halm/platform/lpc/gen_1/can_base.h>
#include <halm/platform/lpc/gen_1/can_defs.h>
#include <halm/pm.h>
#include <halm/timer.h>
#include <xcore/accel.h>
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_CAN_EXACT_RATE
#define MAX_CLOCK_ERROR 0
#else
#define MAX_CLOCK_ERROR UINT32_MAX
#endif
/*----------------------------------------------------------------------------*/
enum Mode
{
  MODE_LISTENER,
  MODE_ACTIVE,
  MODE_LOOPBACK
};
/*----------------------------------------------------------------------------*/
struct Can
{
  struct CanBase base;

  void (*callback)(void *);
  void *callbackArgument;

  /* Timer for the time stamp generation */
  struct Timer *timer;

  /* Message pool */
  PointerArray pool;
  /* Queue for received messages */
  PointerQueue rxQueue;
  /* Queue for transmitting messages */
  PointerQueue txQueue;
  /* Pointer to a memory region used as a message pool */
  void *arena;
  /* Desired baud rate */
  uint32_t rate;
  /* Current interface mode */
  uint8_t mode;
  /* Message sequence number */
  uint8_t sequence;

#ifdef CONFIG_PLATFORM_LPC_CAN_WATERMARK
  /* Maximum available frames in the receive queue */
  size_t rxWatermark;
  /* Maximum pending frames in the transmit queue */
  size_t txWatermark;
#endif

#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
  /* Bus errors */
  uint32_t errorCount;
  /* Received frame overruns */
  uint32_t overrunCount;
  /* Received frames */
  uint32_t rxCount;
  /* Transmitted frames */
  uint32_t txCount;
#endif
};
/*----------------------------------------------------------------------------*/
static bool calcSegments(uint8_t, uint8_t *, uint8_t *);
static bool calcTimings(const struct Can *, uint32_t, uint32_t *);
static uint32_t getBusRate(const struct Can *);
static void interruptHandler(void *);
static void readMessage(struct Can *, struct CANMessage *);
static void resetQueues(struct Can *);
static void sendMessage(struct Can *, const struct CANMessage *, uint32_t *);
static void setBusMode(struct Can *, enum Mode);
static bool setBusRate(struct Can *, uint32_t);
static void updateRxWatermark(struct Can *, size_t);
static void updateTxWatermark(struct Can *, size_t);

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
static bool calcSegments(uint8_t width, uint8_t *tseg1, uint8_t *tseg2)
{
  /* Width of the bit segment 1, the synchronization time segment excluded */
  const uint8_t seg1 = width * CONFIG_PLATFORM_LPC_CAN_SP / 100 - 1;

  if (seg1 > BTR_TSEG1_MAX + 1)
    return false;

  const uint8_t seg2 = width - seg1 - 1;

  if (seg2 > BTR_TSEG2_MAX + 1)
    return false;
  if (seg2 <= CONFIG_PLATFORM_LPC_CAN_SJW)
    return false;
  if (seg2 > seg1 + 1)
    return false;

  *tseg1 = seg1;
  *tseg2 = seg2;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool calcTimings(const struct Can *interface, uint32_t rate,
    uint32_t *result)
{
  static const uint8_t MAX_WIDTH = BTR_TSEG1_MAX + BTR_TSEG2_MAX + 3;
  static const uint8_t MIN_WIDTH = 3;

  if (!rate)
    return false;

  const uint32_t apbClock = canGetClock(&interface->base);
  uint32_t currentError = MAX_CLOCK_ERROR;
  uint32_t currentPrescaler;
  uint8_t currentDelta = UINT8_MAX;
  uint8_t currentSeg1;
  uint8_t currentSeg2;

  for (uint8_t width = MAX_WIDTH; width >= MIN_WIDTH; --width)
  {
    const uint32_t clock = rate * width;
    const uint32_t error = apbClock % clock;

    if (error > currentError)
      continue;
    if (clock > apbClock)
      continue;
    if (apbClock / clock > BTR_BRP_MAX + 1)
      continue;

    uint8_t seg1;
    uint8_t seg2;

    if (calcSegments(width, &seg1, &seg2))
    {
      const uint8_t sp = ((uint16_t)seg1 + 1) * 100 / width;
      const uint8_t delta = abs(CONFIG_PLATFORM_LPC_CAN_SP - (int)sp);

      if (error < currentError || delta < currentDelta)
      {
        currentError = error;
        currentPrescaler = apbClock / clock;
        currentDelta = delta;
        currentSeg1 = seg1;
        currentSeg2 = seg2;

        if (error == 0 && delta == 0)
          break;
      }
    }
  }

  if (currentDelta != UINT8_MAX)
  {
    *result = BTR_BRP(currentPrescaler - 1)
        | BTR_SJW(CONFIG_PLATFORM_LPC_CAN_SJW - 1)
        | BTR_TSEG1(currentSeg1 - 1)
        | BTR_TSEG2(currentSeg2 - 1);

    return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/
static uint32_t getBusRate(const struct Can *interface)
{
  const LPC_CAN_Type * const reg = interface->base.reg;
  const uint32_t apbClock = canGetClock(&interface->base);
  const uint32_t btr = reg->BTR;
  const uint32_t prescaler = BTR_BRP_VALUE(btr) + 1;
  const uint32_t width = BTR_TSEG1_VALUE(btr) + BTR_TSEG2_VALUE(btr) + 3;

  return apbClock / prescaler / width;
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
      else
      {
#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
        ++interface->overrunCount;
#endif
      }

      /* Release receive buffer */
      reg->CMR = CMR_RRB | CMR_CDO;
    }

    updateRxWatermark(interface, pointerQueueSize(&interface->rxQueue));
  }

#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
  if (icr & ICR_DOI)
  {
    ++interface->overrunCount;
  }
#endif

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
      updateTxWatermark(interface, pointerQueueSize(&interface->txQueue));

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

      if (pointerQueueEmpty(&interface->txQueue))
        event = true;
    }
  }

  if (icr & ICR_BEI)
  {
#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
    ++interface->errorCount;
#endif

    if (sr & SR_BS)
    {
      /*
       * The controller is forced into a bus-off state, RM bit should be cleared
       * to continue normal operation.
       */
      reg->MOD &= ~MOD_RM;
    }
  }

  if (event && interface->callback != NULL)
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

#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
  ++interface->rxCount;
#endif
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

#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
  ++interface->txCount;
#endif
}
/*----------------------------------------------------------------------------*/
static void setBusMode(struct Can *interface, enum Mode mode)
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
static bool setBusRate(struct Can *interface, uint32_t rate)
{
  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t btr;

  if (!calcTimings(interface, rate, &btr))
    return false;

  reg->MOD |= MOD_RM; /* Enable Reset mode */
  reg->BTR = btr;
  reg->MOD &= ~MOD_RM;

  return true;
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct Can *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_LPC_CAN_WATERMARK
  if (level > interface->rxWatermark)
    interface->rxWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
static void updateTxWatermark(struct Can *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_LPC_CAN_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_CAN_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct Can * const interface = object;
    setBusRate(interface, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *object, const void *configBase)
{
  const struct CanConfig * const config = configBase;
  assert(config != NULL);

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

  const size_t poolSize = config->rxBuffers + config->txBuffers;

  if (!pointerArrayInit(&interface->pool, poolSize))
    return E_MEMORY;
  if (!pointerQueueInit(&interface->rxQueue, config->rxBuffers))
    return E_MEMORY;
  if (!pointerQueueInit(&interface->txQueue, config->txBuffers))
    return E_MEMORY;

  interface->arena = malloc(sizeof(struct CANStandardMessage) * poolSize);
  if (interface->arena == NULL)
    return E_MEMORY;

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->timer = config->timer;
  interface->mode = MODE_LISTENER;
  interface->rate = config->rate;
  interface->sequence = 0;

#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
  interface->errorCount = 0;
  interface->overrunCount = 0;
  interface->rxCount = 0;
  interface->txCount = 0;
#endif

#ifdef CONFIG_PLATFORM_LPC_CAN_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  struct CANStandardMessage *message = interface->arena;

  for (size_t index = 0; index < poolSize; ++index)
  {
    pointerArrayPushBack(&interface->pool, message);
    ++message;
  }

  LPC_CAN_Type * const reg = interface->base.reg;
  uint32_t btr;

  if (!calcTimings(interface, config->rate, &btr))
    return E_VALUE;

  reg->MOD = MOD_RM; /* Reset CAN */
  reg->IER = 0; /* Disable Receive Interrupt */
  reg->GSR = 0; /* Reset error counter */
  reg->BTR = btr; /* Configure time segments, the bus is sampled once */

  /* Activate Listen Only mode and enable local priority for transmit buffers */
  reg->MOD = MOD_LOM | MOD_TPM;

  /* Configure the Acceptance Filter */
  LPC_CANAF->AFMR = AFMR_AccBP;

#ifdef CONFIG_PLATFORM_LPC_CAN_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable interrupts on message reception and bus error */
  reg->IER = IER_RIE | IER_DOIE | IER_EPIE | IER_BEIE | IER_TIE_MASK;

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

  free(interface->arena);
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

  switch ((enum CANParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
    case IF_CAN_ERRORS:
      *(uint32_t *)data = interface->errorCount;
      break;

    case IF_CAN_OVERRUNS:
      *(uint32_t *)data = interface->overrunCount;
      break;

    case IF_CAN_RX_COUNT:
      *(uint32_t *)data = interface->rxCount;
      break;

    case IF_CAN_TX_COUNT:
      *(uint32_t *)data = interface->txCount;
      break;
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      *(size_t *)data = pointerQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_RX_PENDING:
      *(size_t *)data = pointerQueueCapacity(&interface->rxQueue)
          - pointerQueueSize(&interface->rxQueue);
      return E_OK;

    case IF_TX_AVAILABLE:
      *(size_t *)data = pointerQueueCapacity(&interface->txQueue)
          - pointerQueueSize(&interface->txQueue);
      return E_OK;

    case IF_TX_PENDING:
      *(size_t *)data = pointerQueueSize(&interface->txQueue);
      return E_OK;

#ifdef CONFIG_PLATFORM_LPC_CAN_WATERMARK
    case IF_RX_WATERMARK:
      *(size_t *)data = interface->rxWatermark;
      return E_OK;

    case IF_TX_WATERMARK:
      *(size_t *)data = interface->txWatermark;
      return E_OK;
#endif

    case IF_RATE:
      *(uint32_t *)data = getBusRate(interface);
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
      setBusMode(interface, MODE_ACTIVE);
      return E_OK;

    case IF_CAN_LISTENER:
      setBusMode(interface, MODE_LISTENER);
      return E_OK;

    case IF_CAN_LOOPBACK:
      setBusMode(interface, MODE_LOOPBACK);
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
    {
      const uint32_t rate = *(const uint32_t *)data;

      if (setBusRate(interface, rate))
      {
        interface->rate = rate;
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
