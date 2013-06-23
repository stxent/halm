/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "serial.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_PRIORITY    15 /* Lowest interrupt priority in Cortex-M3 */
#define RX_FIFO_LEVEL       2 /* 8 characters */
#define TX_FIFO_SIZE        8
/*----------------------------------------------------------------------------*/
enum cleanup
{
  FREE_NONE = 0,
  FREE_PARENT,
  FREE_RX_QUEUE,
  FREE_TX_QUEUE,
  FREE_ALL
};
/*----------------------------------------------------------------------------*/
static void cleanup(struct Serial *, enum cleanup);
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct Serial),
    .init = serialInit,
    .deinit = serialDeinit,

    .read = serialRead,
    .write = serialWrite,
    .get = serialGet,
    .set = serialSet
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *Serial = &serialTable;
/*----------------------------------------------------------------------------*/
static void cleanup(struct Serial *device, enum cleanup step)
{
  switch (step)
  {
    case FREE_ALL:
      NVIC_DisableIRQ(device->parent.irq); /* Disable interrupt */
    case FREE_TX_QUEUE:
      queueDeinit(&device->txQueue);
    case FREE_RX_QUEUE:
      queueDeinit(&device->rxQueue);
    case FREE_PARENT:
      Uart->deinit(device); /* Call UART class destructor */
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Serial *device = object;
  uint8_t data;

  /* Interrupt status cleared when performed read operation on IIR register */
  if (device->parent.reg->IIR & IIR_INT_STATUS)
    return;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (device->parent.reg->LSR & LSR_RDR)
  {
    data = device->parent.reg->RBR;
    /* Received bytes will be dropped when queue becomes full */
    if (!queueFull(&device->rxQueue))
      queuePush(&device->rxQueue, data);
  }
  if (device->parent.reg->LSR & LSR_THRE)
  {
    uint8_t counter = 0;

    /* Fill FIFO with selected burst size or less */
    while (!queueEmpty(&device->txQueue) && counter++ < TX_FIFO_SIZE)
      device->parent.reg->THR = queuePop(&device->txQueue);
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SerialConfig * const config = configPtr;
  struct Serial *device = object;
  struct UartConfig parentConfig;
  enum result res;

  /* Check device configuration data */
  assert(config);

  /* Initialize parent configuration structure */
  parentConfig.channel = config->channel;
  parentConfig.rx = config->rx;
  parentConfig.tx = config->tx;
  parentConfig.rate = config->rate;
  parentConfig.parity = config->parity;

  /* Call UART class constructor */
  if ((res = Uart->init(object, &parentConfig)) != E_OK)
    return res;

  /* Set pointer to hardware interrupt handler */
  device->parent.handler = interruptHandler;

  /* Initialize RX and TX queues */
  if ((res = queueInit(&device->rxQueue, config->rxLength)) != E_OK)
  {
    cleanup(device, FREE_PARENT);
    return res;
  }
  if ((res = queueInit(&device->txQueue, config->txLength)) != E_OK)
  {
    cleanup(device, FREE_RX_QUEUE);
    return res;
  }

  device->queueLock = MUTEX_UNLOCKED;

  /* Enable and clear FIFO, set RX trigger level to 8 bytes */
  device->parent.reg->FCR &= ~FCR_RX_TRIGGER_MASK;
  device->parent.reg->FCR |= FCR_ENABLE | FCR_RX_TRIGGER(RX_FIFO_LEVEL);
  /* Enable RBR and THRE interrupts */
  device->parent.reg->IER |= IER_RBR | IER_THRE;
  device->parent.reg->TER = TER_TXEN;

  /* Set interrupt priority, lowest by default */
  NVIC_SetPriority(device->parent.irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  cleanup(object, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object, enum ifOption option, void *data)
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct Serial *device = object;
  enum result res;

  switch (option)
  {
    case IF_RATE:
    {
      struct UartRateConfig rate;

      if ((res = uartCalcRate(&rate, *(uint32_t *)data)) != E_OK)
        return res;
      uartSetRate(object, rate);
      return E_OK;
    }
    case IF_PRIORITY:
      NVIC_SetPriority(device->parent.irq, *(uint32_t *)data);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t serialRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Serial *device = object;
  uint32_t read;

  mutexLock(&device->queueLock);
  NVIC_DisableIRQ(device->parent.irq);
  read = queuePopArray(&device->rxQueue, buffer, length);
  NVIC_EnableIRQ(device->parent.irq);
  mutexUnlock(&device->queueLock);
  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t serialWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct Serial *device = object;
  uint32_t written = 0;

  mutexLock(&device->queueLock);
  /* Check transmitter state */
  if (device->parent.reg->LSR & LSR_TEMT && queueEmpty(&device->txQueue))
  {
    /* Transmitter is idle, fill TX FIFO */
    NVIC_DisableIRQ(device->parent.irq);
    while (written < TX_FIFO_SIZE && written < length)
    {
      device->parent.reg->THR = *buffer++;
      ++written;
    }
    NVIC_EnableIRQ(device->parent.irq);
    length -= written;
  }
  if (length)
  {
    /* Fill TX queue with the rest of data */
    NVIC_DisableIRQ(device->parent.irq);
    written += queuePushArray(&device->txQueue, buffer, length);
    NVIC_EnableIRQ(device->parent.irq);
  }
  mutexUnlock(&device->queueLock);
  return written;
}
