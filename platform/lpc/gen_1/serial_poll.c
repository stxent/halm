/*
 * serial_poll.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/uart_defs.h>
#include <halm/platform/lpc/serial_poll.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define TX_FIFO_SIZE 8
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_UART_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *, const void *);
static enum Result serialGetParam(void *, int, void *);
static enum Result serialSetParam(void *, int, const void *);
static size_t serialRead(void *, void *, size_t);
static size_t serialWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void serialDeinit(void *);
#else
#  define serialDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SerialPoll = &(const struct InterfaceClass){
    .size = sizeof(struct SerialPoll),
    .init = serialInit,
    .deinit = serialDeinit,

    .setCallback = NULL,
    .getParam = serialGetParam,
    .setParam = serialSetParam,
    .read = serialRead,
    .write = serialWrite
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_UART_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct SerialPoll * const interface = object;
    struct UartRateConfig rateConfig;

    /* Recalculate and set baud rate */
    if (uartCalcRate(&interface->base, interface->rate, &rateConfig))
      uartSetRate(&interface->base, rateConfig);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialInit(void *object, const void *configBase)
{
  const struct SerialPollConfig * const config = configBase;
  assert(config != NULL);

  const struct UartBaseConfig baseConfig = {
      .rx = config->rx,
      .tx = config->tx,
      .channel = config->channel
  };
  struct SerialPoll * const interface = object;
  struct UartRateConfig rateConfig;
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!uartCalcRate(&interface->base, config->rate, &rateConfig))
    return E_VALUE;

  interface->rate = config->rate;

  LPC_UART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->LCR = LCR_WLS(WLS_8BIT);
  /* Enable FIFO and reset RX trigger level */
  reg->FCR = (reg->FCR & ~FCR_RXTRIGLVL_MASK) | FCR_FIFOEN
      | FCR_RXTRIGLVL(RX_TRIGGER_LEVEL_1);
  /* Disable all interrupts */
  reg->IER = 0;
  /* Transmitter is enabled by default thus TER register is left untouched */

  uartSetParity(&interface->base, config->parity);
  uartSetRate(&interface->base, rateConfig);

#ifdef CONFIG_PLATFORM_LPC_UART_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_UART_NO_DEINIT
static void serialDeinit(void *object)
{
#ifdef CONFIG_PLATFORM_LPC_UART_PM
  pmUnregister(object);
#endif

  UartBase->deinit(object);
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result serialGetParam(void *object, int parameter, void *data)
{
  const struct SerialPoll * const interface = object;

#ifdef CONFIG_PLATFORM_LPC_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      *(uint8_t *)data = (uint8_t)uartGetParity(&interface->base);
      return E_OK;

    case IF_SERIAL_STOPBITS:
      *(uint8_t *)data = (uint8_t)uartGetStopBits(&interface->base);
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
      *(uint32_t *)data = uartGetRate(&interface->base);
      return E_OK;

    default:
      return E_INVALID;
  }
#else
  (void)object;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif
}
/*----------------------------------------------------------------------------*/
static enum Result serialSetParam(void *object, int parameter, const void *data)
{
  struct SerialPoll * const interface = object;

#ifdef CONFIG_PLATFORM_LPC_UART_RC
  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
    {
      const enum SerialParity parity = *(const uint8_t *)data;

      uartSetParity(&interface->base, parity);
      return E_OK;
    }

    case IF_SERIAL_STOPBITS:
    {
      const enum SerialStopBits number = *(const uint8_t *)data;

      uartSetStopBits(&interface->base, number);
      return E_OK;
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
    {
      struct UartRateConfig rateConfig;
      const uint32_t rate = *(const uint32_t *)data;

      if (uartCalcRate(&interface->base, rate, &rateConfig))
      {
        interface->rate = rate;
        uartSetRate(&interface->base, rateConfig);
        return E_OK;
      }
      else
        return E_VALUE;
    }

    default:
      return E_INVALID;
  }
#else
  (void)interface;
  (void)parameter;
  (void)data;

  return E_INVALID;
#endif
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
      size_t bytesToWrite = MIN(left, TX_FIFO_SIZE);

      written += bytesToWrite;
      while (bytesToWrite--)
        reg->THR = *bufferPosition++;
    }
  }

  return written;
}
