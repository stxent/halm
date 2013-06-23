/*
 * one_wire.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include "one_wire.h"
#include "uart_defs.h"
/*----------------------------------------------------------------------------*/
#define DEFAULT_PRIORITY    15 /* Lowest interrupt priority in Cortex-M3 */
#define RATE_RESET          9600
#define RATE_DATA           115200
/*----------------------------------------------------------------------------*/
enum romCommand
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
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
static void cleanup(struct OneWire *, enum cleanup);
static void interruptHandler(void *);
static enum result resetChannel(struct OneWire *);
static void sendWord(struct OneWire *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *, const void *);
static void oneWireDeinit(void *);
static uint32_t oneWireRead(void *, uint8_t *, uint32_t);
static uint32_t oneWireWrite(void *, const uint8_t *, uint32_t);
static enum result oneWireGet(void *, enum ifOption, void *);
static enum result oneWireSet(void *, enum ifOption, const void *);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct OneWire),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .read = oneWireRead,
    .write = oneWireWrite,
    .get = oneWireGet,
    .set = oneWireSet
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *OneWire = &serialTable;
/*----------------------------------------------------------------------------*/
static void cleanup(struct OneWire *device, enum cleanup step)
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
static enum result resetChannel(struct OneWire *device)
{
  uartSetRate((struct Uart *)device, device->resetRate);
  device->state = OW_RESET;
  /* Clear RX FIFO and set trigger level to 1 character */
  device->parent.reg->FCR |= FCR_RX_RESET | FCR_RX_TRIGGER(0);
  device->parent.reg->THR = 0xF0; /* Execute reset */
  while (device->state == OW_RESET); /* Wait reset to be completed */

  if (device->state == OW_READY)
  {
    uartSetRate((struct Uart *)device, device->dataRate);
    return E_OK;
  }
  else
    return E_DEVICE;
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWire *device, uint8_t word)
{
  uint8_t counter = 0;

  while (counter < 8)
    device->parent.reg->THR = (word >> counter++) & 0x01 ? 0xFF : 0;
//    device->parent.reg->THR = ((word >> counter++) & 0x01) * 0xFF;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct OneWire *device = object;

  /* Interrupt status cleared when performed read operation on IIR register */
  if (device->parent.reg->IIR & IIR_INT_STATUS)
    return;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (device->parent.reg->LSR & LSR_RDR)
  {
    uint8_t data = device->parent.reg->RBR;
    switch (device->state)
    {
      case OW_RESET:
        device->state = data ^ 0xF0 ? OW_READY : OW_IDLE;
        break;
      case OW_RECEIVE:
        if (data & 0x01)
          device->word |= 1 << device->rxPosition;
        if (++device->rxPosition == 8)
        {
          queuePush(&device->rxQueue, device->word);
          device->word = 0x00;
          device->rxPosition = 0;
          if (!--device->left)
            device->state = OW_IDLE;
        }
        break;
      case OW_TRANSMIT:
        if (++device->rxPosition == 8)
        {
          device->rxPosition = 0;
          if (!--device->left)
            device->state = OW_IDLE;
        }
      default:
        break;
    }
  }
  if (device->parent.reg->LSR & LSR_THRE)
  {
    /* Fill FIFO with next word or end the transaction */
    if (!queueEmpty(&device->txQueue))
      sendWord(device, queuePop(&device->txQueue));
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *object, const void *configPtr)
{
  /* Set pointer to device configuration data */
  const struct OneWireConfig * const config = configPtr;
  struct OneWire *device = object;
  struct UartConfig parentConfig;
  enum result res;

  /* Check device configuration data */
  assert(config);

  /* Compute rates */
   if ((res = uartCalcRate(&device->dataRate, RATE_DATA)) != E_OK)
     return res;
   if ((res = uartCalcRate(&device->resetRate, RATE_RESET)) != E_OK)
     return res;

  /* Initialize parent configuration structure */
  parentConfig.channel = config->channel;
  parentConfig.rx = config->rx;
  parentConfig.tx = config->tx;
  parentConfig.rate = RATE_RESET;

  /* Call UART class constructor */
  if ((res = Uart->init(object, &parentConfig)) != E_OK)
    return res;

  gpioSetType(&device->parent.txPin, GPIO_OPENDRAIN);
//  gpioSetPull(&device->parent.txPin, GPIO_PULLUP); //XXX

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

  device->channelLock = MUTEX_UNLOCKED;
  device->state = OW_IDLE;
  device->address.rom = 0;

  /* Enable and clear FIFO, set RX trigger level to 8 bytes */
  device->parent.reg->FCR &= ~FCR_RX_TRIGGER_MASK;
  device->parent.reg->FCR |= FCR_ENABLE;
  /* Enable RBR and THRE interrupts */
  device->parent.reg->IER |= IER_THRE;
  device->parent.reg->IER |= IER_RBR | IER_THRE;

  /* Set interrupt priority, lowest by default */
  NVIC_SetPriority(device->parent.irq, DEFAULT_PRIORITY);
  /* Enable UART interrupt */
  NVIC_EnableIRQ(device->parent.irq);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void oneWireDeinit(void *object)
{
  cleanup(object, FREE_ALL);
}
/*----------------------------------------------------------------------------*/
static enum result oneWireGet(void *object, enum ifOption option, void *data)
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result oneWireSet(void *object, enum ifOption option,
    const void *data)
{
  struct OneWire *device = object;
  enum result res;

  switch (option)
  {
    case IF_DEVICE:
      if ((device->address.rom = *(uint64_t *)data))
        mutexLock(&device->deviceLock);
      else
        mutexUnlock(&device->deviceLock);
      return E_OK;
    case IF_PRIORITY:
      NVIC_SetPriority(device->parent.irq, *(uint32_t *)data);
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct OneWire *device = object;
  uint32_t read;

  if (!length)
    return;

  mutexLock(&device->channelLock);

  device->rxPosition = 0;
  device->left = 1;
  device->word = 0x00;
  while (device->left != length) //TODO Rewrite
  {
    queuePush(&device->txQueue, 0xFF);
    device->left++;
  }

  device->state = OW_RECEIVE;
  /* Clear RX FIFO and set trigger level to 8 characters */
  device->parent.reg->FCR |= FCR_RX_RESET | FCR_RX_TRIGGER(2);
  sendWord(device, 0xFF); /* Start reception */
  while (device->state == OW_RECEIVE);
  read = queuePopArray(&device->rxQueue, buffer, length);

  mutexUnlock(&device->channelLock);

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct OneWire *device = object;
  uint32_t written;

  if (!length)
    return 0;

  mutexLock(&device->channelLock);

  if (resetChannel(device) != E_OK)
  {
    /* No devices detected */
    mutexUnlock(&device->channelLock);
    return 0;
  }

  device->rxPosition = 0;
  device->left = 1;
  device->state = OW_TRANSMIT;
  /* Initiate new transaction by selecting addressing mode */
  if (device->address.rom)
  {
    device->left += queuePushArray(&device->txQueue,
        (const uint8_t *)&device->address.rom, length);
    written = queuePushArray(&device->txQueue, buffer, length);
    device->left += written;
    sendWord(device, (uint8_t)MATCH_ROM);
  }
  else
  {
    written = queuePushArray(&device->txQueue, buffer, length);
    device->left += written;
    sendWord(device, (uint8_t)SKIP_ROM);
  }
  while (device->state == OW_TRANSMIT);

  mutexUnlock(&device->channelLock);

  return written;
}
