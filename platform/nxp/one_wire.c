/*
 * one_wire.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <platform/nxp/one_wire.h>
#include <platform/nxp/uart_defs.h>
/*----------------------------------------------------------------------------*/
#define RATE_RESET      9600
#define RATE_DATA       115200
#define TX_QUEUE_LENGTH 24
/*----------------------------------------------------------------------------*/
enum romCommand
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWire *, const struct OneWireConfig *);
static void beginTransmission(struct OneWire *);
static void interruptHandler(void *);
static void sendWord(struct OneWire *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *, const void *);
static void oneWireDeinit(void *);
static enum result oneWireCallback(void *, void (*)(void *), void *);
static enum result oneWireGet(void *, enum ifOption, void *);
static enum result oneWireSet(void *, enum ifOption, const void *);
static uint32_t oneWireRead(void *, uint8_t *, uint32_t);
static uint32_t oneWireWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct OneWire),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .callback = oneWireCallback,
    .get = oneWireGet,
    .set = oneWireSet,
    .read = oneWireRead,
    .write = oneWireWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass *OneWire = &serialTable;
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWire *interface __attribute__((unused)),
    const struct OneWireConfig *config)
{
  gpioSetType(gpioInit(config->tx), GPIO_OPENDRAIN);
}
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWire *interface)
{
  LPC_UART_Type *reg = interface->parent.reg;

  uartSetRate((struct UartBase *)interface, interface->resetRate);
  interface->state = OW_RESET;
  /* Clear RX FIFO and set trigger level to 1 character */
  reg->FCR |= FCR_RX_RESET | FCR_RX_TRIGGER(0);
  reg->THR = 0xF0; /* Execute reset */
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWire *interface, uint8_t word)
{
  LPC_UART_Type *reg = interface->parent.reg;
  uint8_t counter = 0;

  while (counter < 8)
    reg->THR = (word >> counter++) & 0x01 ? 0xFF : 0;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct OneWire *interface = object;
  LPC_UART_Type *reg = interface->parent.reg;
  bool event = false;

  /* Interrupt status cleared when performed read operation on IIR register */
  if (reg->IIR & IIR_INT_STATUS)
    return;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    uint8_t data = reg->RBR;
    switch (interface->state)
    {
      case OW_RESET:
        if (data ^ 0xF0)
        {
          uartSetRate((struct UartBase *)interface, interface->dataRate);
          interface->state = OW_TRANSMIT;
        }
        else
        {
          interface->state = OW_ERROR;
          event = true;
        }
        break;
      case OW_RECEIVE:
        if (data & 0x01)
          interface->word |= 1 << interface->bit;
      case OW_TRANSMIT:
        if (++interface->bit == 8)
        {
          if (interface->state == OW_RECEIVE)
          {
            *interface->rxBuffer++ = interface->word;
            interface->word = 0x00;
          }
          interface->bit = 0;
          if (!--interface->left)
          {
            interface->state = OW_IDLE;
            event = true;
          }
        }
        break;
      default:
        break;
    }
  }
  if ((reg->LSR & LSR_THRE) && interface->state != OW_RESET)
  {
    /* Fill FIFO with next word or end the transaction */
    if (!byteQueueEmpty(&interface->txQueue))
      sendWord(interface, byteQueuePop(&interface->txQueue));
  }
  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *object, const void *configPtr)
{
  const struct OneWireConfig * const config = configPtr;
  const struct UartBaseConfig parentConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct OneWire *interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &parentConfig)) != E_OK)
    return res;

  adjustPins(interface, config);

  if ((res = byteQueueInit(&interface->txQueue, TX_QUEUE_LENGTH)) != E_OK)
    return res;

  interface->parent.handler = interruptHandler;

  interface->dataRate = uartCalcRate(object, RATE_DATA);
  interface->resetRate = uartCalcRate(object, RATE_RESET);

  interface->address.rom = 0;
  interface->callback = 0;
  interface->blocking = true;
  interface->state = OW_IDLE;

  LPC_UART_Type *reg = interface->parent.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WORD_8BIT;
  /* Enable FIFO and set RX trigger level to single byte */
  reg->FCR = (reg->FCR & ~FCR_RX_TRIGGER_MASK) | FCR_ENABLE;
  /* Enable RBR and THRE interrupts */
  reg->IER = IER_RBR | IER_THRE;
  /* Enable transmitter */
  reg->TER = TER_TXEN;

  uartSetRate(object, interface->resetRate);

  irqSetPriority(interface->parent.irq, config->priority);
  irqEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void oneWireDeinit(void *object)
{
  struct OneWire *interface = object;

  irqDisable(interface->parent.irq);
  byteQueueDeinit(&interface->txQueue);
  UartBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result oneWireCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct OneWire *interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result oneWireGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  struct OneWire *interface = object;

  switch (option)
  {
    case IF_STATUS:
      if (!interface->blocking && interface->state == OW_ERROR)
        return E_ERROR;
      else
        return interface->state != OW_IDLE ? E_BUSY : E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireSet(void *object, enum ifOption option,
    const void *data)
{
  struct OneWire *interface = object;

  switch (option)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;
    case IF_DEVICE:
      interface->address.rom = *(uint64_t *)data;
      return E_OK;
    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct OneWire *interface = object;
  uint32_t read = 0;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->rxBuffer = buffer;
  interface->word = 0x00;
  while (!byteQueueFull(&interface->txQueue) && ++read != length)
    byteQueuePush(&interface->txQueue, 0xFF);
  interface->left = read;

  interface->state = OW_RECEIVE;
  /* Clear RX FIFO and set trigger level to 8 characters */
  ((LPC_UART_Type *)interface->parent.reg)->FCR |= FCR_RX_RESET
      | FCR_RX_TRIGGER(2);
  sendWord(interface, 0xFF); /* Start reception */

  if (interface->blocking)
    while (interface->state != OW_IDLE && interface->state != OW_ERROR);

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct OneWire *interface = object;
  uint32_t written;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->left = 1;
  /* Initiate new transaction by selecting addressing mode */
  if (interface->address.rom)
  {
    byteQueuePush(&interface->txQueue, (uint8_t)MATCH_ROM);
    interface->left += byteQueuePushArray(&interface->txQueue,
        (const uint8_t *)&interface->address.rom, length);
  }
  else
    byteQueuePush(&interface->txQueue, (uint8_t)SKIP_ROM);
  written = byteQueuePushArray(&interface->txQueue, buffer, length);
  interface->left += written;

  beginTransmission(interface);

  if (interface->blocking)
    while (interface->state != OW_IDLE && interface->state != OW_ERROR);

  return written;
}
