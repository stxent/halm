/*
 * serial.c
 * Copyright (C) 2012 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include "uart_defs.h"
#include "serial.h"
/*----------------------------------------------------------------------------*/
/* Serial port settings */
#define TX_FIFO_SIZE                    8
#define RX_FIFO_LEVEL                   2 /* 8 characters */
#define DEFAULT_PRIORITY                15 /* Lowest priority in Cortex-M3 */
/*----------------------------------------------------------------------------*/
enum cleanup
{
  FREE_NONE = 0,
  FREE_PERIPHERAL,
  FREE_RX_QUEUE,
  FREE_TX_QUEUE,
  FREE_ALL
};
/*----------------------------------------------------------------------------*/
static void serialCleanup(struct Serial *, enum cleanup);
/*----------------------------------------------------------------------------*/
static void serialHandler(void *);
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static uint32_t serialRead(void *, uint8_t *, uint32_t);
static uint32_t serialWrite(void *, const uint8_t *, uint32_t);
static enum result serialGetOpt(void *, enum ifOption, void *);
static enum result serialSetOpt(void *, enum ifOption,
    const void *);
/*----------------------------------------------------------------------------*/
/* We like structures so we put a structure in a structure */
/* So we can initialize a structure while we initialize a structure */
static const struct UartClass serialTable = {
    .parent = {
        .size = sizeof(struct Serial),
        .init = serialInit,
        .deinit = serialDeinit,

        .read = serialRead,
        .write = serialWrite,
        .getopt = serialGetOpt,
        .setopt = serialSetOpt
    },
    .handler = serialHandler
};
/*----------------------------------------------------------------------------*/
const struct UartClass *Serial = &serialTable;
/*----------------------------------------------------------------------------*/
static void serialCleanup(struct Serial *device, enum cleanup step)
{
  switch (step)
  {
    case FREE_ALL:
      NVIC_DisableIRQ(device->parent.irq); /* Disable interrupt */
    case FREE_TX_QUEUE:
      queueDeinit(&device->txQueue);
    case FREE_RX_QUEUE:
      queueDeinit(&device->rxQueue);
    case FREE_PERIPHERAL:
      /* Call UART class destructor */
      Uart->parent.deinit(device);
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void serialHandler(void *object)
{
  struct Serial *device = object;
  uint8_t counter = 0, data;

  /* Interrupt status cleared when performed read operation on IIR register */
  switch (device->parent.reg->IIR & IIR_INT_MASK)
  {
    case IIR_INT_RDA:
    case IIR_INT_CTI:
      /* Byte will be removed from FIFO after reading from RBR register */
      while (device->parent.reg->LSR & LSR_RDR)
      {
        data = device->parent.reg->RBR;
        /* Received bytes will be dropped when queue becomes full */
        if (!queueFull(&device->rxQueue))
          queuePush(&device->rxQueue, data);
      }
      break;
    case IIR_INT_THRE:
      /* Fill FIFO with selected burst size or less */
      while (queueSize(&device->txQueue) && counter++ < TX_FIFO_SIZE)
        device->parent.reg->THR = queuePop(&device->txQueue);
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialGetOpt(void *object, enum ifOption option, void *data)
{
  struct Serial *device = object;

  switch (option)
  {
    case IF_SPEED:
      /* TODO */
      return E_OK;
    case IF_QUEUE_RX:
      *(uint32_t *)data = queueSize(&device->rxQueue);
      return E_OK;
    case IF_QUEUE_TX:
      *(uint32_t *)data = queueSize(&device->txQueue);
      return E_OK;
    case IF_PRIORITY:
      *(uint32_t *)data = NVIC_GetPriority(device->parent.irq);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSetOpt(void *object, enum ifOption option,
    const void *data)
{
  struct Serial *device = object;

  switch (option)
  {
    case IF_SPEED:
      return uartSetRate(object, uartCalcRate(*(uint32_t *)data));
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
  uint32_t read = 0;

  mutexLock(&device->queueLock);
  NVIC_DisableIRQ(device->parent.irq);
  while (queueSize(&device->rxQueue) && read < length)
  {
    *buffer++ = queuePop(&device->rxQueue);
    read++;
  }
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
  NVIC_DisableIRQ(device->parent.irq);
  /* Check transmitter state */
  if (device->parent.reg->LSR & LSR_TEMT && queueEmpty(&device->txQueue))
  {
    /* Transmitter is idle, fill TX FIFO */
    while (written < TX_FIFO_SIZE && written < length)
    {
      device->parent.reg->THR = *buffer++;
      written++;
    }
  }
  /* Fill TX queue with the rest of data */
  while (!queueFull(&device->txQueue) && written < length)
  {
    queuePush(&device->txQueue, *buffer++);
    written++;
  }
  NVIC_EnableIRQ(device->parent.irq);
  mutexUnlock(&device->queueLock);
  return written;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
  struct Serial *device = object;

  /* Release resources */
  serialCleanup(device, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SerialConfig *config = configPtr;
  struct Serial *device = object;
  enum result res;

  /* Call UART class constructor */
  if ((res = Uart->parent.init(object, configPtr)) != E_OK)
    return res;

  /* Initialize RX and TX queues */
  if ((res = queueInit(&device->rxQueue, config->rxLength)) != E_OK)
  {
    serialCleanup(device, FREE_PERIPHERAL);
    return res;
  }

  if ((res = queueInit(&device->txQueue, config->txLength)) != E_OK)
  {
    serialCleanup(device, FREE_RX_QUEUE);
    return res;
  }

  device->queueLock = MUTEX_UNLOCKED;

  if ((res = uartSetRate(object, uartCalcRate(config->rate))) != E_OK)
  {
    serialCleanup(device, FREE_TX_QUEUE);
    return res;
  }

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
