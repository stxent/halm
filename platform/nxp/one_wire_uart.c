/*
 * one_wire_uart.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <memory.h>
#include <platform/nxp/one_wire_uart.h>
#include <platform/nxp/uart_defs.h>
/*----------------------------------------------------------------------------*/
#define RATE_RESET      9600
#define RATE_DATA       115200
#define TX_QUEUE_LENGTH 24
/*----------------------------------------------------------------------------*/
enum oneWireRomCommand
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireUart *, const struct OneWireUartConfig *);
static void beginTransmission(struct OneWireUart *);
static void interruptHandler(void *);
static void sendWord(struct OneWireUart *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum result oneWireInit(void *, const void *);
static void oneWireDeinit(void *);
static enum result oneWireCallback(void *, void (*)(void *), void *);
static enum result oneWireGet(void *, enum ifOption, void *);
static enum result oneWireSet(void *, enum ifOption, const void *);
static uint32_t oneWireRead(void *, uint8_t *, uint32_t);
static uint32_t oneWireWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass oneWireTable = {
    .size = sizeof(struct OneWireUart),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .callback = oneWireCallback,
    .get = oneWireGet,
    .set = oneWireSet,
    .read = oneWireRead,
    .write = oneWireWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const OneWireUart = &oneWireTable;
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireUart *interface __attribute__((unused)),
    const struct OneWireUartConfig *config)
{
  gpioSetType(gpioInit(config->tx), GPIO_OPENDRAIN);
}
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWireUart *interface)
{
  LPC_UART_Type * const reg = interface->parent.reg;

  uartSetRate((struct UartBase *)interface, interface->resetRate);
  interface->state = OW_UART_RESET;
  /* Clear RX FIFO and set trigger level to 1 character */
  reg->FCR |= FCR_RX_RESET | FCR_RX_TRIGGER(0);
  reg->THR = 0xF0; /* Execute reset */
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWireUart *interface, uint8_t word)
{
  LPC_UART_Type * const reg = interface->parent.reg;
  uint8_t counter = 0;

  while (counter < 8)
    reg->THR = (word >> counter++) & 0x01 ? 0xFF : 0;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct OneWireUart * const interface = object;
  LPC_UART_Type * const reg = interface->parent.reg;
  bool event = false;

  /* Interrupt status cleared when performed read operation on IIR register */
  if (reg->IIR & IIR_INT_STATUS)
    return;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    const uint8_t data = reg->RBR;

    switch (interface->state)
    {
      case OW_UART_RESET:
        if (data ^ 0xF0)
        {
          uartSetRate((struct UartBase *)interface, interface->dataRate);
          interface->state = OW_UART_TRANSMIT;
        }
        else
        {
          interface->state = OW_UART_ERROR;
          event = true;
        }
        break;

      case OW_UART_RECEIVE:
        if (data & 0x01)
          interface->word |= 1 << interface->bit;
      case OW_UART_TRANSMIT:
        if (++interface->bit == 8)
        {
          if (interface->state == OW_UART_RECEIVE)
          {
            *interface->rxBuffer++ = interface->word;
            interface->word = 0x00;
          }
          interface->bit = 0;
          if (!--interface->left)
          {
            interface->state = OW_UART_IDLE;
            event = true;
          }
        }
        break;

      default:
        break;
    }
  }

  if ((reg->LSR & LSR_THRE) && (interface->state == OW_UART_RECEIVE
      || interface->state == OW_UART_TRANSMIT))
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
  const struct OneWireUartConfig * const config = configPtr;
  const struct UartBaseConfig parentConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct OneWireUart * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &parentConfig)) != E_OK)
    return res;

  adjustPins(interface, config);

  if ((res = byteQueueInit(&interface->txQueue, TX_QUEUE_LENGTH)) != E_OK)
    return res;

  if ((res = uartCalcRate(object, RATE_DATA, &interface->dataRate)) != E_OK)
    return res;
  if ((res = uartCalcRate(object, RATE_RESET, &interface->resetRate)) != E_OK)
    return res;

  interface->parent.handler = interruptHandler;

  interface->address = 0;
  interface->blocking = true;
  interface->callback = 0;
  interface->state = OW_UART_IDLE;

  LPC_UART_Type * const reg = interface->parent.reg;

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
  struct OneWireUart * const interface = object;

  irqDisable(interface->parent.irq);
  byteQueueDeinit(&interface->txQueue);
  UartBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result oneWireCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct OneWireUart * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result oneWireGet(void *object, enum ifOption option,
    void *data __attribute__((unused)))
{
  struct OneWireUart * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      if (!interface->blocking && interface->state == OW_UART_ERROR)
        return E_ERROR;
      else
        return interface->state != OW_UART_IDLE ? E_BUSY : E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result oneWireSet(void *object, enum ifOption option,
    const void *data)
{
  struct OneWireUart * const interface = object;

  switch (option)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_DEVICE:
      interface->address = toLittleEndian64(*(const uint64_t *)data);
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
  struct OneWireUart * const interface = object;
  LPC_UART_Type * const reg = interface->parent.reg;
  uint32_t read = 0;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->rxBuffer = buffer;
  interface->word = 0x00;

  /* Fill queue with dummy words */
  while (!byteQueueFull(&interface->txQueue) && ++read != length)
    byteQueuePush(&interface->txQueue, 0xFF);
  interface->left = read;

  interface->state = OW_UART_RECEIVE;
  /* Clear RX FIFO and set trigger level to 8 characters */
  reg->FCR |= FCR_RX_RESET | FCR_RX_TRIGGER(2);
  sendWord(interface, 0xFF); /* Start reception */

  if (interface->blocking)
  {
    while (interface->state != OW_UART_IDLE
        && interface->state != OW_UART_ERROR)
    {
      barrier();
    }

    if (interface->state == OW_UART_ERROR)
      return 0;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static uint32_t oneWireWrite(void *object, const uint8_t *buffer,
    uint32_t length)
{
  struct OneWireUart * const interface = object;
  uint32_t written;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->left = 1;
  /* Initiate new transaction by selecting the addressing mode */
  if (interface->address)
  {
    byteQueuePush(&interface->txQueue, (uint8_t)MATCH_ROM);
    interface->left += byteQueuePushArray(&interface->txQueue,
        (const uint8_t *)&interface->address, length);
  }
  else
    byteQueuePush(&interface->txQueue, (uint8_t)SKIP_ROM);
  written = byteQueuePushArray(&interface->txQueue, buffer, length);
  interface->left += written;

  beginTransmission(interface);

  if (interface->blocking)
  {
    while (interface->state != OW_UART_IDLE
        && interface->state != OW_UART_ERROR)
    {
      barrier();
    }

    if (interface->state == OW_UART_ERROR)
      return 0;
  }

  return written;
}
