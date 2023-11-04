/*
 * can.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/can.h>
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/numicro/can.h>
#include <halm/platform/numicro/can_base.h>
#include <halm/platform/numicro/can_defs.h>
#include <halm/pm.h>
#include <halm/timer.h>
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_EXACT_RATE
#define MAX_CLOCK_ERROR 0
#else
#define MAX_CLOCK_ERROR UINT32_MAX
#endif

#define MAX_MESSAGES  32

#define RX_COUNT      (MAX_MESSAGES / 2)
#define RX_OBJECT     1
#define RX_REG_INDEX  ((RX_OBJECT - 1) / 16)
#define TX_OBJECT     (1 + RX_COUNT)
#define TX_REG_INDEX  ((TX_OBJECT - 1) / 16)
/*----------------------------------------------------------------------------*/
enum Mode
{
  MODE_LISTENER,
  MODE_ACTIVE,
  MODE_LOOPBACK
};
/*----------------------------------------------------------------------------*/
typedef struct
{
  uint32_t id;
  uint32_t mask;
} CanFilterEntry;

enum
{
  CAN_FILTER_ENTRY_EXT = 1UL << 29
};

DEFINE_ARRAY(CanFilterEntry, Filter, filter)
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

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
  /* Filter entries */
  FilterArray filters;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_WATERMARK
  /* Maximum available frames in the receive queue */
  size_t rxWatermark;
  /* Maximum pending frames in the transmit queue */
  size_t txWatermark;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_COUNTERS
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
static void buildAcceptanceFilters(struct Can *);
static bool calcSegments(uint8_t, uint8_t *, uint8_t *);
static bool calcTimings(const struct Can *, uint32_t, uint32_t *, uint32_t *);
static void dropMessage(struct Can *, size_t);
static uint32_t getBusRate(const struct Can *);
static void interruptHandler(void *);
static void invalidateMessageObject(struct Can *, size_t);
static void listenForMessage(struct Can *, size_t, uint32_t, uint32_t);
static void readMessage(struct Can *, struct CANMessage *, size_t);
static void setBusMode(struct Can *, enum Mode);
static bool setBusRate(struct Can *, uint32_t);
static void updateRxWatermark(struct Can *, size_t);
static void updateTxWatermark(struct Can *, size_t);
static void writeMessage(struct Can *, const struct CANMessage *, size_t, bool);

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
static bool filterAdd(struct Can *, const struct CANFilter *, bool);
static bool filterRemove(struct Can *, const struct CANFilter *, bool);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);
static void canDeinit(void *);
static void canSetCallback(void *, void (*)(void *), void *);
static enum Result canGetParam(void *, int, void *);
static enum Result canSetParam(void *, int, const void *);
static size_t canRead(void *, void *, size_t);
static size_t canWrite(void *, const void *, size_t);
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
static void buildAcceptanceFilters(struct Can *interface)
{
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
  if (!filterArrayEmpty(&interface->filters))
  {
    const size_t width = RX_COUNT / filterArraySize(&interface->filters);
    size_t offset = RX_OBJECT;

    for (size_t rule = 0; rule < filterArraySize(&interface->filters); ++rule)
    {
      const CanFilterEntry entry = *filterArrayAt(&interface->filters, rule);

      for (size_t index = 0; index < width; ++index)
      {
        invalidateMessageObject(interface, index + offset);
        listenForMessage(interface, index + offset, entry.id, entry.mask);
      }

      offset += width;
    }

    for (; offset < TX_OBJECT; ++offset)
      invalidateMessageObject(interface, offset);

    return;
  }
#endif

  for (size_t index = RX_OBJECT; index < TX_OBJECT; ++index)
  {
    /* Accept all messages, disable FIFO mode */
    invalidateMessageObject(interface, index);
    listenForMessage(interface, index, 0, 0);
  }
}
/*----------------------------------------------------------------------------*/
static bool calcSegments(uint8_t width, uint8_t *tseg1, uint8_t *tseg2)
{
  /* Width of the bit segment 1, the synchronization time segment excluded */
  const uint8_t seg1 = width * CONFIG_PLATFORM_NUMICRO_CAN_SP / 100 - 1;

  if (seg1 < BTIME_TSEG1_MIN + 1 || seg1 > BTIME_TSEG1_MAX + 1)
    return false;

  const uint8_t seg2 = width - seg1 - 1;

  if (seg2 > BTIME_TSEG2_MAX + 1)
    return false;
  if (seg2 <= CONFIG_PLATFORM_NUMICRO_CAN_SJW)
    return false;
  if (seg2 > seg1 + 1)
    return false;

  *tseg1 = seg1;
  *tseg2 = seg2;

  return true;
}
/*----------------------------------------------------------------------------*/
static bool calcTimings(const struct Can *interface, uint32_t rate,
    uint32_t *result, uint32_t *extension)
{
  static const uint8_t MAX_WIDTH = BTIME_TSEG1_MAX + BTIME_TSEG2_MAX + 3;
  static const uint8_t MIN_WIDTH = BTIME_TSEG1_MIN + 3;

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
    if (apbClock / clock > BRP_BRPE_MAX + 1)
      continue;

    uint8_t seg1;
    uint8_t seg2;

    if (calcSegments(width, &seg1, &seg2))
    {
      const uint8_t sp = ((uint16_t)seg1 + 1) * 100 / width;
      const uint8_t delta = abs(CONFIG_PLATFORM_NUMICRO_CAN_SP - (int)sp);

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
    *result = (BTIME_BRP(currentPrescaler - 1) & BTIME_BRP_MASK)
        | BTIME_SJW(CONFIG_PLATFORM_NUMICRO_CAN_SJW - 1)
        | BTIME_TSEG1(currentSeg1 - 1)
        | BTIME_TSEG2(currentSeg2 - 1);
    *extension = (currentPrescaler - 1) >> BTIME_BRP_WIDTH;

    return true;
  }

  return false;
}
/*----------------------------------------------------------------------------*/
static void dropMessage(struct Can *interface, size_t index)
{
  NM_CAN_Type * const reg = interface->base.reg;

  /* Clear pending interrupt and new data flag  */
  reg->IF[0].CMASK = CMASK_NEWDAT | CMASK_CLRINTPND;
  reg->IF[0].CREQ = index;
  while (reg->IF[0].CREQ & CREQ_BUSY);

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_COUNTERS
  ++interface->overrunCount;
#endif
}
/*----------------------------------------------------------------------------*/
static uint32_t getBusRate(const struct Can *interface)
{
  const NM_CAN_Type * const reg = interface->base.reg;
  const uint32_t apbClock = canGetClock(&interface->base);
  const uint32_t btime = reg->BTIME;
  const uint32_t prescaler = BTIME_BRP_VALUE(btime) + 1;
  const uint32_t width =
      BTIME_TSEG1_VALUE(btime) + BTIME_TSEG2_VALUE(btime) + 3;

  return apbClock / prescaler / width;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Can * const interface = object;
  NM_CAN_Type * const reg = interface->base.reg;
  uint32_t id;
  bool event = false;

  while ((id = IIDR_INTID_VALUE(reg->IIDR)) != INTID_NONE)
  {
    const uint32_t status = reg->STATUS;

    /* Clear all flags, update Last Error Code */
    reg->STATUS = STATUS_LEC(LEC_UNUSED);

    if (id == INTID_STATUS)
    {
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_COUNTERS
      const uint8_t error = STATUS_LEC_VALUE(status);

      if (error != LEC_NO_ERROR && error != LEC_UNUSED)
      {
        ++interface->errorCount;
      }
#endif

      if (status & STATUS_BOFF)
      {
        /* Exit bus-off state */
        reg->CON &= ~CON_INIT;
      }
    }
    else if (id < TX_OBJECT)
    {
      /* Receive messages */
      while (reg->NDAT[RX_REG_INDEX] & (1UL << (id - 1)))
      {
        const uint32_t timestamp = interface->timer ?
            timerGetValue(interface->timer) : 0;

        if (!pointerQueueFull(&interface->rxQueue))
        {
          struct CANMessage * const message =
              pointerArrayBack(&interface->pool);
          pointerArrayPopBack(&interface->pool);

          readMessage(interface, message, id);
          message->timestamp = timestamp;
          pointerQueuePushBack(&interface->rxQueue, message);
        }
        else
        {
          /* Received message will be lost when queue is full */
          dropMessage(interface, id);
        }

        ++id;
      }

      updateRxWatermark(interface, pointerQueueSize(&interface->rxQueue));
      event = true;
    }
    else
    {
      /* Clear pending transmit interrupt */
      invalidateMessageObject(interface, id);
    }
  }

  if (!pointerQueueEmpty(&interface->txQueue) && !reg->TXREQ[TX_REG_INDEX])
  {
    const size_t pendingMessages = pointerQueueSize(&interface->txQueue);
    const size_t lastMessageIndex = MIN(pendingMessages, MAX_MESSAGES / 2);

    updateTxWatermark(interface, pendingMessages);

    for (size_t index = 0; index < lastMessageIndex; ++index)
    {
      struct CANMessage * const message =
          pointerQueueFront(&interface->txQueue);
      pointerQueuePopFront(&interface->txQueue);

      writeMessage(interface, message, TX_OBJECT + index,
          index == lastMessageIndex - 1);
      pointerArrayPushBack(&interface->pool, message);
    }

    if (pointerQueueEmpty(&interface->txQueue))
      event = true;
  }

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void invalidateMessageObject(struct Can *interface, size_t index)
{
  NM_CAN_Type * const reg = interface->base.reg;

  /* Clear Message Valid flag */
  reg->IF[0].ARB1 = 0;
  reg->IF[0].ARB2 = 0;

  /* Disable reception and transmission interrupts, clear transmit request */
  reg->IF[0].MCON = 0;

  reg->IF[0].CMASK = CMASK_WR | CMASK_CONTROL | CMASK_ARB;
  reg->IF[0].CREQ = index;

  /* Wait until read/write action is finished */
  while (reg->IF[0].CREQ & CREQ_BUSY);
}
/*----------------------------------------------------------------------------*/
static void listenForMessage(struct Can *interface, size_t index, uint32_t id,
    uint32_t mask)
{
  NM_CAN_Type * const reg = interface->base.reg;

  reg->IF[0].MCON = MCON_EOB | MCON_UMASK | MCON_RXIE | MCON_DLC(8);

  uint16_t msk1 = 0;
  uint16_t msk2 = 0;

  if (mask != 0)
  {
    msk2 = MASK2_MXTD;

    if (id & CAN_FILTER_ENTRY_EXT)
    {
      msk1 = MASK1_EXT_ID_FROM_MASK(mask);
      msk2 |= MASK2_EXT_ID_FROM_MASK(mask);
    }
    else
      msk2 |= MASK2_STD_ID_FROM_MASK(mask);
  }

  reg->IF[0].MASK1 = msk1;
  reg->IF[0].MASK2 = msk2;

  uint16_t arb1 = 0;
  uint16_t arb2 = ARB2_MSGVAL;

  if (id & CAN_FILTER_ENTRY_EXT)
  {
    arb1 = ARB1_EXT_ID_FROM_ID(id);
    arb2 |= ARB2_EXT_ID_FROM_ID(id) | ARB2_XTD;
  }
  else
    arb2 |= ARB2_STD_ID_FROM_ID(id);

  reg->IF[0].ARB1 = arb1;
  reg->IF[0].ARB2 = arb2;

  /* Active the Message Object */
  reg->IF[0].CMASK = CMASK_WR | CMASK_CONTROL | CMASK_ARB | CMASK_MASK;
  reg->IF[0].CREQ = index;
  while (reg->IF[0].CREQ & CREQ_BUSY);
}
/*----------------------------------------------------------------------------*/
static void readMessage(struct Can *interface, struct CANMessage *message,
    size_t index)
{
  NM_CAN_Type * const reg = interface->base.reg;

  /* Read from Message Object to system memory and clear interrupt flag */
  reg->IF[0].CMASK = CMASK_DATA_A | CMASK_DATA_B | CMASK_CONTROL | CMASK_ARB;
  reg->IF[0].CREQ = index;
  while (reg->IF[0].CREQ & CREQ_BUSY);

  const uint32_t arb1 = reg->IF[0].ARB1;
  const uint32_t arb2 = reg->IF[0].ARB2;
  const uint32_t control = reg->IF[0].MCON;

  /* Clear flags */
  reg->IF[0].MCON = MCON_EOB | MCON_UMASK | MCON_RXIE | MCON_DLC(8);
  reg->IF[0].CMASK = CMASK_WR | CMASK_CONTROL;
  reg->IF[0].CREQ = index;
  while (reg->IF[0].CREQ & CREQ_BUSY);

  /* Fill message structure */
  message->flags = 0;
  message->length = MCON_DLC_VALUE(control);

  if (arb2 & ARB2_DIR)
  {
    message->flags |= CAN_RTR;
  }
  else
  {
    const uint16_t data[] = {
        reg->IF[0].DAT_A1,
        reg->IF[0].DAT_A2,
        reg->IF[0].DAT_B1,
        reg->IF[0].DAT_B2
    };

    memcpy(message->data, data, sizeof(data));
  }

  if (arb2 & ARB2_XTD)
  {
    message->id = EXT_ID_FROM_ARB(arb1, arb2);
    message->flags |= CAN_EXT_ID;
  }
  else
    message->id = ARB2_STD_ID_VALUE(arb2);

#ifdef CONFIG_PLATFORM_LPC_CAN_COUNTERS
  if (control & MCON_MSGLST)
    ++interface->overrunCount;
  ++interface->rxCount;
#endif
}
/*----------------------------------------------------------------------------*/
static void setBusMode(struct Can *interface, enum Mode mode)
{
  NM_CAN_Type * const reg = interface->base.reg;
  uint32_t control = reg->CON | CON_TEST;
  uint32_t test = 0;

  /* Enable write access to the Test register */
  reg->CON = control;

  switch (mode)
  {
    case MODE_LISTENER:
      test = TEST_SILENT;
      break;

    case MODE_LOOPBACK:
      test = TEST_SILENT | TEST_LBACK;
      break;

    default:
      control &= ~CON_TEST;
      break;
  }

  /*
   * Test functions are only active when the TEST bit
   * in the Control register is set.
   */
  reg->TEST = test;
  reg->CON = control;
}
/*----------------------------------------------------------------------------*/
static bool setBusRate(struct Can *interface, uint32_t rate)
{
  NM_CAN_Type * const reg = interface->base.reg;
  const uint32_t state = reg->CON;
  uint32_t brpe;
  uint32_t btime;

  if (!calcTimings(interface, rate, &btime, &brpe))
    return false;

  reg->CON = state | (CON_INIT | CON_CCE);
  reg->BRPE = brpe;
  reg->BTIME = btime;
  reg->CON = state;

  return true;
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct Can *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_WATERMARK
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
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
static void writeMessage(struct Can *interface,
    const struct CANMessage *message, size_t index, bool last)
{
  NM_CAN_Type * const reg = interface->base.reg;

  /* Prepare values for the message interface registers */
  uint32_t arb1;
  uint32_t arb2 = ARB2_MSGVAL;
  uint32_t control = MCON_EOB | MCON_TXRQST | MCON_NEWDAT;
  uint32_t mask = CMASK_WR | CMASK_CONTROL | CMASK_ARB;

  if (last)
    control |= MCON_TXIE;

  if (!(message->flags & CAN_RTR))
  {
    uint16_t data[4];

    /* Message is not a Remote Transmission Request */
    arb2 |= ARB2_DIR;

    /* Fill data registers */
    control |= MCON_DLC(message->length);
    memcpy(data, message->data, sizeof(data));

    reg->IF[0].DAT_A1 = data[0];
    reg->IF[0].DAT_A2 = data[1];
    reg->IF[0].DAT_B1 = data[2];
    reg->IF[0].DAT_B2 = data[3];

    /* Issue copying of data to Message Object */
    mask |= CMASK_DATA_A | CMASK_DATA_B;
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

    arb1 = 0;
    arb2 |= ARB2_STD_ID_FROM_ID(message->id);
  }

  reg->IF[0].MCON = control;

  reg->IF[0].ARB1 = arb1;
  reg->IF[0].ARB2 = arb2;

  reg->IF[0].CMASK = mask;
  reg->IF[0].CREQ = index;

  /* Wait until read/write action is finished */
  while (reg->IF[0].CREQ & CREQ_BUSY);

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_COUNTERS
  ++interface->txCount;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
static bool filterAdd(struct Can *interface, const struct CANFilter *filter,
    bool ext)
{
  if (filterArrayFull(&interface->filters))
    return false;

  CanFilterEntry entry;

  if (ext)
  {
    entry.id = (filter->id & MASK(29)) | CAN_FILTER_ENTRY_EXT;
    entry.mask = filter->mask & MASK(29);
  }
  else
  {
    entry.id = filter->id & MASK(11);
    entry.mask = filter->mask & MASK(11);
  }

  filterArrayPushBack(&interface->filters, entry);
  return true;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
static bool filterRemove(struct Can *interface, const struct CANFilter *filter,
    bool ext)
{
  CanFilterEntry target;
  bool matched = false;

  if (ext)
  {
    target.id = (filter->id & MASK(29)) | CAN_FILTER_ENTRY_EXT;
    target.mask = filter->mask & MASK(29);
  }
  else
  {
    target.id = filter->id & MASK(11);
    target.mask = filter->mask & MASK(11);
  }

  for (size_t index = 0; index < filterArraySize(&interface->filters);)
  {
    const CanFilterEntry entry = *filterArrayAt(&interface->filters, index);

    if (entry.id == target.id && entry.mask == target.mask)
    {
      filterArrayErase(&interface->filters, index);
      matched = true;
    }
    else
      ++index;
  }

  return matched;
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_PM
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
  assert(config->filters <= RX_COUNT);

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

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
  if (!filterArrayInit(&interface->filters, config->filters))
    return E_MEMORY;
#endif

  interface->base.handler = interruptHandler;
  interface->callback = NULL;
  interface->rate = config->rate;
  interface->timer = config->timer;

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_COUNTERS
  interface->errorCount = 0;
  interface->overrunCount = 0;
  interface->rxCount = 0;
  interface->txCount = 0;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  struct CANStandardMessage *message = interface->arena;

  for (size_t index = 0; index < poolSize; ++index)
  {
    pointerArrayPushBack(&interface->pool, message);
    ++message;
  }

  NM_CAN_Type * const reg = interface->base.reg;

  /* Enable write access to the Test register and enter initialization mode */
  reg->CON = CON_INIT | CON_TEST;
  /* Enable listen only mode */
  reg->TEST = TEST_SILENT;

  /* Configure bit timings */
  if (!setBusRate(interface, interface->rate))
    return E_VALUE;

  /* All Message Objects should be reset manually */
  for (size_t index = 1; index <= MAX_MESSAGES; ++index)
    invalidateMessageObject(interface, index);

  /* Prepare RX Message Objects */
  buildAcceptanceFilters(interface);

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable interrupts and exit initialization mode */
  reg->CON = (reg->CON & ~CON_INIT) | (CON_IE | CON_EIE);

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void canDeinit(void *object)
{
  struct Can * const interface = object;
  NM_CAN_Type * const reg = interface->base.reg;

  irqDisable(interface->base.irq);
  reg->CON = CON_INIT;

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_PM
  pmUnregister(interface);
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
  filterArrayDeinit(&interface->filters);
#endif

  free(interface->arena);
  pointerQueueDeinit(&interface->txQueue);
  pointerQueueDeinit(&interface->rxQueue);
  pointerArrayDeinit(&interface->pool);

  CanBase->deinit(interface);
}
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
#ifdef CONFIG_PLATFORM_NUMICRO_CAN_COUNTERS
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

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_WATERMARK
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

#ifdef CONFIG_PLATFORM_NUMICRO_CAN_FILTERS
    case IF_CAN_FILTER_ADD_STD:
      if (filterAdd(interface, data, false))
      {
        buildAcceptanceFilters(interface);
        return E_OK;
      }
      else
        return E_FULL;

    case IF_CAN_FILTER_REMOVE_STD:
      if (filterRemove(interface, data, false))
      {
        buildAcceptanceFilters(interface);
        return E_OK;
      }
      else
        return E_VALUE;

    case IF_CAN_FILTER_ADD_EXT:
      if (filterAdd(interface, data, true))
      {
        buildAcceptanceFilters(interface);
        return E_OK;
      }
      else
        return E_FULL;

    case IF_CAN_FILTER_REMOVE_EXT:
      if (filterRemove(interface, data, true))
      {
        buildAcceptanceFilters(interface);
        return E_OK;
      }
      else
        return E_VALUE;
#endif

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
  NM_CAN_Type * const reg = interface->base.reg;
  const size_t totalMessages = length / sizeof(struct CANStandardMessage);
  const struct CANStandardMessage *current = buffer;
  const struct CANStandardMessage * const last = current + totalMessages;

  /* Synchronize access to the message queue and the CAN core */
  irqDisable(interface->base.irq);

  if (pointerQueueEmpty(&interface->txQueue) && !reg->TXREQ[TX_REG_INDEX])
  {
    const size_t messagesToWrite = MIN(totalMessages, MAX_MESSAGES / 2);

    for (size_t index = 0; index < messagesToWrite; ++index)
    {
      writeMessage(interface, (const struct CANMessage *)(current + index),
          TX_OBJECT + index, index == (messagesToWrite - 1));
    }

    current += messagesToWrite;
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
