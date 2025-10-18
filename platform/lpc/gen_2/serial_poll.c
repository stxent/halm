/*
 * serial_poll.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_2/uart_defs.h>
#include <halm/platform/lpc/serial_poll.h>
#include <halm/pm.h>
#include <assert.h>
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
    uartSetRate(&interface->base, interface->rate);
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
  enum Result res;

  /* Call base class constructor */
  if ((res = UartBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->rate = config->rate;

  LPC_USART_Type * const reg = interface->base.reg;

  /* Set 8-bit length */
  reg->CFG = CFG_DATALEN(DATALEN_8BIT);
  /* Configure oversampling value */
  reg->OSR = OSR_OSRVAL(OSR_OSRVAL_MAX);

  if (!uartSetRate(&interface->base, config->rate))
    return E_VALUE;
  uartSetParity(&interface->base, config->parity);

  /* Enable the peripheral */
  reg->CFG |= CFG_ENABLE;

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
  struct SerialPoll * const interface = object;
  LPC_USART_Type * const reg = interface->base.reg;

  reg->CFG = 0;

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
      const uint32_t rate = *(const uint32_t *)data;

      if (uartSetRate(&interface->base, rate))
      {
        interface->rate = rate;
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
  LPC_USART_Type * const reg = interface->base.reg;
  uint8_t *bufferPosition = buffer;
  size_t read = 0;

  /* Byte will be removed from FIFO after reading from RBR register */
  while (read < length && (reg->STAT & STAT_RXRDY))
  {
    *bufferPosition++ = reg->RXDATA;
    ++read;
  }

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t serialWrite(void *object, const void *buffer, size_t length)
{
  struct SerialPoll * const interface = object;
  LPC_USART_Type * const reg = interface->base.reg;
  const uint8_t * const bufferEnd = (const uint8_t *)buffer + length;
  const uint8_t *bufferPosition = buffer;

  while (bufferPosition != bufferEnd)
  {
    /* Check transmitter state */
    if (reg->STAT & STAT_TXRDY)
      reg->TXDATA = *bufferPosition++;
  }

  return length;
}
