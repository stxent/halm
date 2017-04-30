/*
 * serial_poll.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <halm/platform/nxp/gen_1/uart_defs.h>
#include <halm/platform/nxp/serial_poll.h>
#include <halm/pm.h>
/*----------------------------------------------------------------------------*/
#define TX_FIFO_SIZE 8
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static void serialDeinit(void *);
static enum Result serialSetCallback(void *, void (*)(void *), void *);
static enum Result serialGetParam(void *, enum IfParameter, void *);
static enum Result serialSetParam(void *, enum IfParameter, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass serialTable = {
    .size = sizeof(struct SerialPoll),
    .init = serialInit,
    .deinit = serialDeinit,

    .setCallback = serialSetCallback,
    .getParam = serialGetParam,
    .setParam = serialSetParam,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SerialPoll = &serialTable;
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NXP_UART_PM
static void powerStateHandler(void *object, enum PmState state)
{
  struct SerialPoll * const interface = object;

  if (state == PM_ACTIVE)
  {
    struct UartRateConfig rateConfig;

    /* Recalculate and set baud rate */
    if (uartCalcRate(object, interface->rate, &rateConfig))
      uartSetRate(object, rateConfig);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialPollConfig * const config = configBase;
  assert(config);

  const struct UartBaseConfig baseConfig = {
      .channel = config->channel,
      .rx = config->rx,
      .tx = config->tx
  };
  struct SerialPoll * const interface = object;
  struct UartRateConfig rateConfig;
  enum Result res;

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

#ifdef CONFIG_PLATFORM_NXP_UART_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void serialDeinit(void *object)
{
#ifdef CONFIG_PLATFORM_NXP_UART_PM
  pmUnregister(object);
#endif

  UartBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static enum Result serialSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object __attribute__((unused)),
    enum IfParameter parameter __attribute__((unused)),
    void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static enum Result serialSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct SerialPoll * const interface = object;

  switch (parameter)
  {
    case IF_RATE:
    {
      struct UartRateConfig rateConfig;
      const enum Result res = uartCalcRate(object, *(const uint32_t *)data,
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
