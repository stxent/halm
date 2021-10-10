/*
 * can.c
 * Copyright (C) 2018 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/can.h>
#include <halm/generic/pointer_array.h>
#include <halm/generic/pointer_queue.h>
#include <halm/platform/stm32/bxcan_base.h>
#include <halm/platform/stm32/bxcan_defs.h>
#include <halm/platform/stm32/can.h>
#include <halm/pm.h>
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
  struct BxCanBase base;

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

#ifdef CONFIG_PLATFORM_STM32_CAN_WATERMARK
  /* Maximum available frames in the receive queue */
  size_t rxWatermark;
  /* Maximum pending frames in the transmit queue */
  size_t txWatermark;
#endif
};
/*----------------------------------------------------------------------------*/
static uint32_t calcBusTimings(const struct Can *, uint32_t);
static void changeMode(struct Can *, enum Mode);
static void changeRate(struct Can *, uint32_t);
static void interruptHandler(void *);
static void readMessage(struct Can *, struct CANMessage *, unsigned int);
static void sendMessage(struct Can *, const struct CANMessage *);
static void setupAcceptanceFilter(struct Can *, unsigned int, unsigned int,
    bool, uint32_t, uint32_t);
static void updateRxWatermark(struct Can *, size_t);
static void updateTxWatermark(struct Can *, size_t);

#ifdef CONFIG_PLATFORM_STM32_CAN_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result canInit(void *, const void *);
static void canSetCallback(void *, void (*)(void *), void *);
static enum Result canGetParam(void *, int, void *);
static enum Result canSetParam(void *, int, const void *);
static size_t canRead(void *, void *, size_t);
static size_t canWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_STM32_CAN_NO_DEINIT
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
      1 + CONFIG_PLATFORM_STM32_CAN_TSEG1 + CONFIG_PLATFORM_STM32_CAN_TSEG2;
  const uint32_t clock = canGetClock(&interface->base);
  const uint32_t prescaler = clock / rate / bitsPerFrame;
  const uint32_t regValue = BTR_BRP(prescaler - 1) | BTR_SJW(0)
      | BTR_TS1(CONFIG_PLATFORM_STM32_CAN_TSEG1 - 1)
      | BTR_TS2(CONFIG_PLATFORM_STM32_CAN_TSEG2 - 1);

  return regValue;
}
/*----------------------------------------------------------------------------*/
static void changeMode(struct Can *interface, enum Mode mode)
{
  STM_CAN_Type * const reg = interface->base.reg;
  uint32_t value = reg->BTR;

  switch (mode)
  {
    case MODE_LISTENER:
      value = (value & ~BTR_LBKM) | BTR_SILM;
      break;

    case MODE_LOOPBACK:
      value |= BTR_LBKM | BTR_SILM;
      break;

    default:
      value &= ~(BTR_LBKM | BTR_SILM);
      break;
  }

  if (value != reg->BTR)
  {
    /* Enter initialization mode */
    reg->MCR |= MCR_INRQ;
    while (!(reg->MSR & MSR_INAK));

    reg->BTR = value;

    /* Leave initialization mode */
    reg->MCR &= ~MCR_INRQ;
  }
}
/*----------------------------------------------------------------------------*/
static void changeRate(struct Can *interface, uint32_t rate)
{
  STM_CAN_Type * const reg = interface->base.reg;

  /* Enter initialization mode */
  reg->MCR |= MCR_INRQ;
  while (!(reg->MSR & MSR_INAK));

  reg->BTR = calcBusTimings(interface, rate);

  /* Request exit from initialization mode */
  reg->MCR &= ~MCR_INRQ;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Can * const interface = object;
  STM_CAN_Type * const reg = interface->base.reg;
  bool event = false;
  bool queued = false;

  /* Read received messages, use FIFO0 only */
  while (RF_FMP_VALUE(reg->RFR[0]) > 0)
  {
    const uint32_t timestamp = interface->timer ?
        timerGetValue(interface->timer) : 0;

    if (!pointerQueueFull(&interface->rxQueue))
    {
      struct CANMessage * const message = pointerArrayBack(&interface->pool);
      pointerArrayPopBack(&interface->pool);

      readMessage(interface, message, 0);
      message->timestamp = timestamp;
      pointerQueuePushBack(&interface->rxQueue, message);
      event = true;
    }

    reg->RFR[0] |= RF_RFOM;
  }

  updateRxWatermark(interface, pointerQueueSize(&interface->rxQueue));
  updateTxWatermark(interface, pointerQueueSize(&interface->txQueue));

  /* Write pending messages */
  while (!pointerQueueEmpty(&interface->txQueue) && (reg->TSR & TSR_TME_MASK))
  {
    struct CANMessage * const message = pointerQueueFront(&interface->txQueue);
    pointerQueuePopFront(&interface->txQueue);

    sendMessage(interface, message);
    pointerArrayPushBack(&interface->pool, message);

    queued = true;
  }

  if (!queued && (reg->TSR & TSR_TME_MASK) == TSR_TME_MASK
      && pointerQueueEmpty(&interface->txQueue))
  {
    reg->IER &= ~IER_TMEIE;
  }

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void readMessage(struct Can *interface, struct CANMessage *message,
    unsigned int fifo)
{
  STM_CAN_Type * const reg = interface->base.reg;
  const uint32_t rir = reg->RX_FIFO[fifo].RIR;

  message->flags = 0;
  message->length = RDT_DLC_VALUE(reg->RX_FIFO[fifo].RDTR);

  if (rir & RI_IDE)
  {
    message->id = RI_EXID_VALUE(rir);
    message->flags |= CAN_EXT_ID;
  }
  else
  {
    message->id = RI_STID_VALUE(rir);
  }

  if (rir & RI_RTR)
  {
    message->flags |= CAN_RTR;
  }
  else
  {
    const uint32_t data[2] = {
        reg->RX_FIFO[fifo].RDLR,
        reg->RX_FIFO[fifo].RDHR
    };

    /*
     * It is allowed to copy all eight bytes because user space application
     * allocate memory for a whole message structure.
     */
    memcpy(message->data, data, sizeof(data));
  }
}
/*----------------------------------------------------------------------------*/
static void sendMessage(struct Can *interface,
    const struct CANMessage *message)
{
  assert(message->length <= 8);

  STM_CAN_Type * const reg = interface->base.reg;
  const uint32_t mailbox = TSR_CODE_VALUE(reg->TSR);

  if (message->flags & CAN_EXT_ID)
  {
    reg->TX[mailbox].TIR = TI_IDE | TI_EXID(message->id);
  }
  else
  {
    reg->TX[mailbox].TIR = TI_STID(message->id);
  }

  if (message->flags & CAN_RTR)
    reg->TX[mailbox].TIR |= TI_RTR;

  reg->TX[mailbox].TDTR &= ~TDT_DLC_MASK;
  reg->TX[mailbox].TDTR |= TDT_DLC(message->length);

  if (!(message->flags & CAN_RTR))
  {
    uint32_t data[2];
    memcpy(data, message->data, sizeof(data));

    reg->TX[mailbox].TDLR = data[0];
    reg->TX[mailbox].TDHR = data[1];
  }

  /* Start transmission */
  reg->TX[mailbox].TIR |= TI_TXRQ;
}
/*----------------------------------------------------------------------------*/
static void setupAcceptanceFilter(struct Can *interface, unsigned int number,
    unsigned int fifo, bool enabled, uint32_t filterValue, uint32_t filterMask)
{
  STM_CAN_Type * const reg = interface->base.reg;
  const uint32_t mask = BIT(number);

  /* Enter filter initialization mode */
  reg->FMR |= FMR_FINIT;
  /* Deactivate the filter */
  reg->FA1R &= ~mask;

  if (enabled)
  {
    reg->FS1R |= mask;
    reg->FM1R &= ~mask;
    reg->FILTERS[number].FR1 = filterValue;
    reg->FILTERS[number].FR2 = filterMask;

    if (fifo)
      reg->FFA1R |= mask;
    else
      reg->FFA1R &= ~mask;

    /* Activate the filter */
    reg->FA1R |= mask;
  }

  /* Leave filter initialization mode */
  reg->FMR &= ~FMR_FINIT;
}
/*----------------------------------------------------------------------------*/
static void updateRxWatermark(struct Can *interface, size_t level)
{
#ifdef CONFIG_PLATFORM_STM32_CAN_WATERMARK
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
#ifdef CONFIG_PLATFORM_STM32_CAN_WATERMARK
  if (level > interface->txWatermark)
    interface->txWatermark = level;
#else
  (void)interface;
  (void)level;
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_CAN_PM
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

  const struct BxCanBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct Can * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = BxCanBase->init(interface, &baseConfig)) != E_OK)
    return res;

  const size_t poolSize = config->rxBuffers + config->txBuffers;

  if (!pointerArrayInit(&interface->pool, poolSize))
    return E_MEMORY;
  if (!pointerQueueInit(&interface->rxQueue, config->rxBuffers))
    return E_MEMORY;
  if (!pointerQueueInit(&interface->txQueue, config->txBuffers))
    return E_MEMORY;

  interface->arena = malloc(sizeof(struct CANStandardMessage) * poolSize);
  if (!interface->arena)
    return E_MEMORY;

  interface->base.handler = interruptHandler;
  interface->callback = 0;
  interface->timer = config->timer;

#ifdef CONFIG_PLATFORM_STM32_CAN_WATERMARK
  interface->rxWatermark = 0;
  interface->txWatermark = 0;
#endif

  struct CANStandardMessage *message = interface->arena;

  for (size_t index = 0; index < poolSize; ++index)
  {
    pointerArrayPushBack(&interface->pool, message);
    ++message;
  }

  STM_CAN_Type * const reg = interface->base.reg;

  /* Exit from sleep mode */
  reg->MCR &= ~MCR_SLEEP;
  /* Enter initialization mode */
  reg->MCR |= MCR_INRQ;
  while (!(reg->MSR & MSR_INAK));

  reg->MCR = (reg->MCR & ~(MCR_TTCM | MCR_NART))
      | (MCR_ABOM | MCR_AWUM | MCR_RFLM | MCR_TXFP);

  /* Calculate bus timings and enable listener mode */
  reg->BTR = calcBusTimings(interface, config->rate) | BTR_SILM;

  /* Leave initialization mode */
  reg->MCR &= ~MCR_INRQ;

  /* Configure filters: accept all incoming messages */
  setupAcceptanceFilter(interface, 0, 0, true, 0, 0);

#ifdef CONFIG_PLATFORM_STM32_CAN_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable FIFO0 message pending interrupt */
  reg->IER = IER_FMPIE0;

  if (interface->base.irq.tx != interface->base.irq.rx0)
  {
    irqSetPriority(interface->base.irq.tx, config->priority);
    irqEnable(interface->base.irq.tx);
  }

  irqSetPriority(interface->base.irq.rx0, config->priority);
  irqEnable(interface->base.irq.rx0);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_CAN_NO_DEINIT
static void canDeinit(void *object)
{
  struct Can * const interface = object;
  STM_CAN_Type * const reg = interface->base.reg;

  /* Disable all interrupts */
  irqDisable(interface->base.irq.rx0);
  irqDisable(interface->base.irq.tx);
  reg->IER = 0;

#ifdef CONFIG_PLATFORM_STM32_CAN_PM
  pmUnregister(interface);
#endif

  free(interface->arena);
  pointerQueueDeinit(&interface->txQueue);
  pointerQueueDeinit(&interface->rxQueue);
  pointerArrayDeinit(&interface->pool);

  BxCanBase->deinit(interface);
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
    case IF_RX_AVAILABLE:
      *(size_t *)data = pointerQueueSize(&interface->rxQueue)
          * sizeof(struct CANStandardMessage);
      return E_OK;

    case IF_RX_PENDING:
      *(size_t *)data = (pointerQueueCapacity(&interface->rxQueue)
          - pointerQueueSize(&interface->rxQueue))
          * sizeof(struct CANStandardMessage);
      return E_OK;

    case IF_TX_AVAILABLE:
      *(size_t *)data = (pointerQueueCapacity(&interface->txQueue)
          - pointerQueueSize(&interface->txQueue))
          * sizeof(struct CANStandardMessage);
      return E_OK;

    case IF_TX_PENDING:
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

    irqDisable(interface->base.irq.rx0);
    pointerQueuePopFront(&interface->rxQueue);
    pointerArrayPushBack(&interface->pool, input);
    irqEnable(interface->base.irq.rx0);

    ++current;
  }

  return (uintptr_t)current - (uintptr_t)buffer;
}
/*----------------------------------------------------------------------------*/
static size_t canWrite(void *object, const void *buffer, size_t length)
{
  assert(length % sizeof(struct CANStandardMessage) == 0);

  struct Can * const interface = object;
  STM_CAN_Type * const reg = interface->base.reg;
  const struct CANStandardMessage *current = buffer;
  const struct CANStandardMessage * const last =
      (const void *)((uintptr_t)buffer + length);

  irqDisable(interface->base.irq.tx);

  if (pointerQueueEmpty(&interface->txQueue)
      && (reg->TSR & TSR_TME_MASK) == TSR_TME_MASK)
  {
    reg->IER |= IER_TMEIE;

    while (current < last && (reg->TSR & TSR_TME_MASK) != 0)
    {
      /* One of transmit buffers is empty, write new message into it */
      sendMessage(interface, (const struct CANMessage *)current);
      ++current;
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

  irqEnable(interface->base.irq.tx);

  return (uintptr_t)current - (uintptr_t)buffer;
}
