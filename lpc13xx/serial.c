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
static void serialHandler(struct Uart *);
static enum result serialInit(struct Interface *, const void *);
static void serialDeinit(struct Interface *);
static unsigned int serialRead(struct Interface *, uint8_t *, unsigned int);
static unsigned int serialWrite(struct Interface *, const uint8_t *,
    unsigned int);
static enum result serialGetOpt(struct Interface *, enum ifOption, void *);
static enum result serialSetOpt(struct Interface *, enum ifOption,
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
      Uart->parent.deinit((struct Interface *)device);
      break;
    default:
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void serialHandler(struct Uart *base)
{
  struct Serial *device = (struct Serial *)base;
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
static enum result serialGetOpt(struct Interface *iface, enum ifOption option,
    void *data)
{
  struct Serial *device = (struct Serial *)iface;

  switch (option)
  {
    case IF_SPEED:
      /* TODO */
      return E_OK;
    case IF_QUEUE_RX:
      *(uint8_t *)data = queueSize(&device->rxQueue);
      return E_OK;
    case IF_QUEUE_TX:
      *(uint8_t *)data = queueSize(&device->txQueue);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result serialSetOpt(struct Interface *iface, enum ifOption option,
    const void *data)
{
  struct Serial *device = (struct Serial *)iface;

  switch (option)
  {
    case IF_SPEED:
      return uartSetRate((struct Uart *)device,
          uartCalcRate(*(uint32_t *)data));
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static unsigned int serialRead(struct Interface *iface, uint8_t *buffer,
    unsigned int length)
{
  struct Serial *device = (struct Serial *)iface;
  unsigned int read = 0;

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
static unsigned int serialWrite(struct Interface *iface, const uint8_t *buffer,
    unsigned int length)
{
  struct Serial *device = (struct Serial *)iface;
  unsigned int written = 0;

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
static void serialDeinit(struct Interface *iface)
{
  struct Serial *device = (struct Serial *)iface;

  /* Release resources */
  serialCleanup(device, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static enum result serialInit(struct Interface *iface, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct SerialConfig *config = (const struct SerialConfig *)configPtr;
  struct Serial *device = (struct Serial *)iface;

  /* Call UART class constructor */
  if (Uart->parent.init(iface, configPtr) != E_OK)
    return E_ERROR;

  /* Initialize RX and TX queues */
  if (queueInit(&device->rxQueue, config->rxLength) != E_OK)
  {
    serialCleanup(device, FREE_PERIPHERAL);
    return E_ERROR;
  }

  if (queueInit(&device->txQueue, config->txLength) != E_OK)
  {
    serialCleanup(device, FREE_RX_QUEUE);
    return E_ERROR;
  }

  device->queueLock = MUTEX_UNLOCKED;

  if (uartSetRate((struct Uart *)device, uartCalcRate(config->rate)) != E_OK)
  {
    serialCleanup(device, FREE_TX_QUEUE);
    return E_ERROR;
  }

  /* Set 8-bit length */
  device->parent.reg->LCR = LCR_WORD_8BIT;
  /* Enable and clear FIFO, set RX trigger level to 8 bytes */
  device->parent.reg->FCR = FCR_ENABLE | FCR_RX_TRIGGER(RX_FIFO_LEVEL);
  /* Enable RBR and THRE interrupts */
  device->parent.reg->IER = IER_RBR | IER_THRE;
  device->parent.reg->TER = TER_TXEN;

  /* Enable UART interrupt and set priority, lowest by default */
  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
