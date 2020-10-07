/*
 * one_wire_uart.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/gen_1/uart_defs.h>
#include <halm/platform/nxp/one_wire_uart.h>
#include <xcore/memory.h>
/*----------------------------------------------------------------------------*/
#define RATE_RESET      9600
#define RATE_DATA       115200
#define TX_QUEUE_LENGTH 24
/*----------------------------------------------------------------------------*/
enum Command
{
  SEARCH_ROM  = 0xF0,
  READ_ROM    = 0x33,
  MATCH_ROM   = 0x55,
  SKIP_ROM    = 0xCC
};
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_RESET,
  STATE_RECEIVE,
  STATE_TRANSMIT,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireUart *, const struct OneWireUartConfig *);
static void beginTransmission(struct OneWireUart *);
static void interruptHandler(void *);
static void sendWord(struct OneWireUart *, uint8_t);
/*----------------------------------------------------------------------------*/
static enum Result oneWireInit(void *, const void *);
static void oneWireSetCallback(void *, void (*)(void *), void *);
static enum Result oneWireGetParam(void *, enum IfParameter, void *);
static enum Result oneWireSetParam(void *, enum IfParameter, const void *);
static size_t oneWireRead(void *, void *, size_t);
static size_t oneWireWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NXP_UART_NO_DEINIT
static void oneWireDeinit(void *);
#else
#define oneWireDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const OneWireUart =
    &(const struct InterfaceClass){
    .size = sizeof(struct OneWireUart),
    .init = oneWireInit,
    .deinit = oneWireDeinit,

    .setCallback = oneWireSetCallback,
    .getParam = oneWireGetParam,
    .setParam = oneWireSetParam,
    .read = oneWireRead,
    .write = oneWireWrite
};
/*----------------------------------------------------------------------------*/
static void adjustPins(struct OneWireUart *interface __attribute__((unused)),
    const struct OneWireUartConfig *config)
{
  pinSetType(pinInit(config->tx), PIN_OPENDRAIN);
}
/*----------------------------------------------------------------------------*/
static void beginTransmission(struct OneWireUart *interface)
{
  LPC_UART_Type * const reg = interface->base.reg;

  uartSetRate(&interface->base, interface->resetRate);
  interface->state = STATE_RESET;
  /* Clear RX FIFO and set trigger level to 1 character */
  reg->FCR |= FCR_RXFIFORES | FCR_RXTRIGLVL(RX_TRIGGER_LEVEL_1);
  reg->THR = 0xF0; /* Execute reset */
}
/*----------------------------------------------------------------------------*/
static void sendWord(struct OneWireUart *interface, uint8_t word)
{
  LPC_UART_Type * const reg = interface->base.reg;
  uint8_t counter = 0;

  while (counter < 8)
    reg->THR = ((word >> counter++) & 0x01) ? 0xFF : 0;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct OneWireUart * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;
  bool event = false;

  /* Interrupt status cleared when performed read operation on IIR register */
  if (reg->IIR & IIR_INTSTATUS)
    return;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (reg->LSR & LSR_RDR)
  {
    const uint8_t data = reg->RBR;

    switch ((enum State)interface->state)
    {
      case STATE_RESET:
        if (data ^ 0xF0)
        {
          uartSetRate(&interface->base, interface->dataRate);
          interface->state = STATE_TRANSMIT;
        }
        else
        {
          interface->state = STATE_ERROR;
          event = true;
        }
        break;

      case STATE_RECEIVE:
        if (data & 0x01)
          interface->word |= 1 << interface->bit;
        /* Falls through */
      case STATE_TRANSMIT:
        if (++interface->bit == 8)
        {
          if (interface->state == STATE_RECEIVE)
          {
            *interface->rxBuffer++ = interface->word;
            interface->word = 0x00;
          }
          interface->bit = 0;
          if (!--interface->left)
          {
            interface->state = STATE_IDLE;
            event = true;
          }
        }
        break;

      default:
        break;
    }
  }

  if ((reg->LSR & LSR_THRE) && (interface->state == STATE_RECEIVE
      || interface->state == STATE_TRANSMIT))
  {
    /* Fill FIFO with next word or end the transaction */
    if (!byteQueueEmpty(&interface->txQueue))
      sendWord(interface, byteQueuePopFront(&interface->txQueue));
  }

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result oneWireInit(void *object, const void *configBase)
{
  const struct OneWireUartConfig * const config = configBase;
  assert(config);

  const struct UartBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct OneWireUart * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &baseConfig)) != E_OK)
    return res;

  adjustPins(interface, config);

  if (!byteQueueInit(&interface->txQueue, TX_QUEUE_LENGTH))
    return E_MEMORY;

  if ((res = uartCalcRate(object, RATE_DATA, &interface->dataRate)) != E_OK)
    return res;
  if ((res = uartCalcRate(object, RATE_RESET, &interface->resetRate)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  interface->address = 0;
  interface->blocking = true;
  interface->callback = 0;
  interface->state = STATE_IDLE;

  LPC_UART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WLS(WLS_8BIT);
  /* Enable FIFO and set RX trigger level to single byte */
  reg->FCR = (reg->FCR & ~FCR_RXTRIGLVL_MASK) | FCR_FIFOEN;
  /* Enable RBR and THRE interrupts */
  reg->IER = IER_RBRINTEN | IER_THREINTEN;
  /* Transmitter is enabled by default */

  uartSetRate(object, interface->resetRate);

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_UART_NO_DEINIT
static void oneWireDeinit(void *object)
{
  struct OneWireUart * const interface = object;

  irqDisable(interface->base.irq);
  byteQueueDeinit(&interface->txQueue);
  UartBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void oneWireSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct OneWireUart * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result oneWireGetParam(void *object, enum IfParameter parameter,
    void *data __attribute__((unused)))
{
  struct OneWireUart * const interface = object;

  switch (parameter)
  {
    case IF_STATUS:
      if (interface->blocking || interface->state != STATE_ERROR)
        return interface->state != STATE_IDLE ? E_BUSY : E_OK;
      else
        return E_ERROR;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result oneWireSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct OneWireUart * const interface = object;

  switch (parameter)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ADDRESS:
      interface->address = toLittleEndian64(*(const uint64_t *)data);
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t oneWireRead(void *object, void *buffer, size_t length)
{
  struct OneWireUart * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;
  uint8_t read = 0;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->rxBuffer = buffer;
  interface->word = 0x00;

  /* Fill queue with dummy words */
  while (!byteQueueFull(&interface->txQueue) && ++read != length)
    byteQueuePushBack(&interface->txQueue, 0xFF);
  interface->left = read;

  /* Set current mode */
  interface->state = STATE_RECEIVE;

  /* Clear RX FIFO and set trigger level to 8 characters */
  reg->FCR |= FCR_RXFIFORES | FCR_RXTRIGLVL(RX_TRIGGER_LEVEL_8);

  /* Start reception */
  sendWord(interface, 0xFF);

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t oneWireWrite(void *object, const void *buffer, size_t length)
{
  struct OneWireUart * const interface = object;

  if (!length)
    return 0;

  byteQueueClear(&interface->txQueue);
  interface->bit = 0;
  interface->left = 1;

  /* Select the addressing mode */
  if (interface->address)
  {
    byteQueuePushBack(&interface->txQueue, MATCH_ROM);
    interface->left += byteQueuePushArray(&interface->txQueue,
        &interface->address, sizeof(interface->address));
  }
  else
  {
    byteQueuePushBack(&interface->txQueue, SKIP_ROM);
  }

  /* Push data into the transmit queue */
  const size_t written = byteQueuePushArray(&interface->txQueue,
      buffer, length);
  interface->left += written;

  /* Start sending */
  beginTransmission(interface);

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return written;
}
