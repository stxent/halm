/*
 * serial_poll.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/gen_1/uart_defs.h>
#include <halm/platform/nxp/serial_poll.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
#define TX_FIFO_SIZE 8
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_UART_PM
static enum result powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum result serialCallback(void *, void (*)(void *), void *);
static enum result serialGet(void *, enum ifOption, void *);
static enum result serialSet(void *, enum ifOption, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct SerialPoll),
    .init = serialInit,
    .deinit = serialDeinit,

    .callback = serialCallback,
    .get = serialGet,
    .set = serialSet,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SerialPoll = &serialTable;
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_UART_PM
static enum result powerStateHandler(void *object, enum pmState state)
{
  struct SerialPoll * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  if (state == PM_ACTIVE)
  {
    /* Recalculate and set baud rate */
    if ((res = uartCalcRate(object, interface->rate, &rateConfig)) != E_OK)
      return res;

    uartSetRate(object, rateConfig);
  }

  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
static enum result serialInit(void *object, const void *configBase)
{
  const struct SerialPollConfig * const config = configBase;
  const struct UartBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct SerialPoll * const interface = object;
  struct UartRateConfig rateConfig;
  enum result res;

  /* Call base class constructor */
  if ((res = UartBase->init(object, &baseConfig)) != E_OK)
    return res;

  if ((res = uartCalcRate(object, config->rate, &rateConfig)) != E_OK)
    return res;

  interface->rate = config->rate;

  LPC_UART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WORD_8BIT;
  /* Enable FIFO and reset RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RX_TRIGGER_MASK) | FCR_ENABLE
      | FCR_RX_TRIGGER(RX_TRIGGER_LEVEL_1);
  /* Disable all interrupts */
  reg->IER = 0;
  /* Transmitter is enabled by default thus TER register is left untouched */

  uartSetParity(object, config->parity);
  uartSetRate(object, rateConfig);

#ifdef CONFIG_UART_PM
  if ((res = pmRegister(interface, powerStateHandler)) != E_OK)
    return res;
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
#ifdef CONFIG_UART_PM
  pmUnregister(object);
#endif

  UartBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum result serialCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result serialGet(void *object __attribute__((unused)),
    enum ifOption option __attribute__((unused)),
    void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum result serialSet(void *object, enum ifOption option,
    const void *data)
{
  struct SerialPoll * const interface = object;

  switch (option)
  {
    case IF_RATE:
    {
      struct UartRateConfig rateConfig;
      const enum result res = uartCalcRate(object, *(const uint32_t *)data,
          &rateConfig);

      if (res == E_OK)
      {
        interface->rate = *(const uint32_t *)data;
        uartSetRate(object, rateConfig);
      }
      return res;
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t serialRead(void *object, void *buffer, size_t length)
{
  struct SerialPoll * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;
  uint8_t *bufferPosition = buffer;
  size_t read = 0;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (read < length && (reg->LSR & LSR_RDR))
  {
    *bufferPosition++ = reg->RBR;
    ++read;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct SerialPoll * const interface = object;
  LPC_UART_Type * const reg = interface->base.reg;
  const uint8_t *bufferPosition = buffer;
  size_t written = 0;

  while (written < length)
  {
    /* Check transmitter state */
    if (reg->LSR & LSR_THRE)
    {
      const size_t left = length - written;
      unsigned int count = left < TX_FIFO_SIZE ? left : TX_FIFO_SIZE;

      written += count;
      while (count--)
        reg->THR = *bufferPosition++;
    }
  }

  return written;
}
