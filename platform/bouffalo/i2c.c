/*
 * halm/platform/bouffalo/i2c.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/bouffalo/i2c.h>
#include <halm/platform/bouffalo/i2c_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
/* Maximum length of an I2C packet (in bytes) */
#define I2C_PACKET_LENGTH_MAX 255
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);
static void i2cSetCallback(void *, void (*)(void *), void *);
static enum Result i2cGetParam(void *, int, void *);
static enum Result i2cSetParam(void *, int, const void *);
static size_t i2cRead(void *, void *, size_t);
static size_t i2cWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_BOUFFALO_I2C_NO_DEINIT
static void i2cDeinit(void *);
#else
#  define i2cDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const I2C = &(const struct InterfaceClass){
    .size = sizeof(struct I2C),
    .init = i2cInit,
    .deinit = i2cDeinit,

    .setCallback = i2cSetCallback,
    .getParam = i2cGetParam,
    .setParam = i2cSetParam,
    .read = i2cRead,
    .write = i2cWrite
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2C * const interface = object;
  BL_I2C_Type * const reg = interface->base.reg;
  const uint32_t status = reg->I2C_INT_STS;
  bool event = false;

  /* Handle reception */
  if (status & INT_STS_RXFINT)
  {
    const size_t received = FIFO_CONFIG_1_RFICNT_VALUE(reg->I2C_FIFO_CONFIG_1);

    for (size_t index = 0; index < received; ++index)
    {
      const uint32_t word = reg->I2C_FIFO_RDATA;

      for (size_t byte = 0; byte < 4; ++byte)
      {
        if (!interface->rxLeft)
          break;

        *(uint8_t *)interface->buffer = (uint8_t)(word >> (byte * 8));
        interface->buffer++;
        --interface->rxLeft;
      }
    }

    /* Clear the RX FIFO under/overflow flags */
    reg->I2C_FIFO_CONFIG_0 = FIFO_CONFIG_0_RFICLR;
  }

  /* Handle transmission */
  if (status & INT_STS_TXFINT)
  {
    /*
     * The TX FIFO is only 2 words deep. Never write more words than the
     * available space, otherwise the hardware drops them and the FIFO
     * overflow flag is set, which corrupts the data on the bus.
     */
    const size_t space = FIFO_CONFIG_1_TFICNT_VALUE(reg->I2C_FIFO_CONFIG_1);
    size_t pending = MIN(space, (interface->txLeft + 3) / 4);

    while (pending--)
    {
      uint32_t word = 0;
     
      for (size_t byte = 0; byte < 4; ++byte)
      {
        if (interface->txLeft)
        {
          word |= (uint32_t)*(const uint8_t *)interface->buffer << (byte * 8);
          interface->buffer++;
          --interface->txLeft;
        }
      }

      reg->I2C_FIFO_WDATA = word;
    }
  }

  /* Handle errors */
  if (status & (INT_STS_NAKINT | INT_STS_ARBINT | INT_STS_FERINT))
  {
    /*
     * After the last data byte of a read the master generates the
     * protocol final NAK and the transaction ends. This is not an error.
     */
    if ((status & INT_STS_NAKINT) && (status & INT_STS_ENDINT)
        && interface->reading && !interface->rxLeft)
    {
      reg->I2C_INT_STS = INT_STS_NAKCLR;
    }
    else
    {
      /*
       * Aborting the transfer de-asserts the master function and stops
       * the current transaction. The pending counters are dropped so
       * that the interface status reports the error instead of staying
       * busy forever.
       */
      reg->I2C_CONFIG &= ~CONFIG_MEN;
      reg->I2C_INT_STS = INT_STS_NAKCLR | INT_STS_ARBCLR;
      reg->I2C_FIFO_CONFIG_0 = FIFO_CONFIG_0_TFICLR | FIFO_CONFIG_0_RFICLR;
      interface->error = true;
      interface->active = false;
      interface->rxLeft = 0;
      interface->txLeft = 0;
      event = true;
    }
  }

  /* Handle transfer end */
  if (status & INT_STS_ENDINT)
  {
    /*
     * Aborting the transfer de-asserts the master function and stops
     * the current transaction. The interface is ready to accept a new
     * command only after the stop condition has been issued.
     */
    reg->I2C_CONFIG &= ~CONFIG_MEN;
    reg->I2C_INT_STS = INT_STS_ENDCLR;
    interface->active = false;
    event = true;
  }

  if (!interface->active)
    irqDisable(interface->base.irq);

  if (event && interface->callback != nullptr)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct I2C * const interface = object;
    i2cSetRate(&interface->base, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CConfig * const config = configBase;
  assert(config != nullptr);

  const struct I2CBaseConfig baseConfig = {
      .channel = config->channel,
      .scl = config->scl,
      .sda = config->sda
  };
  struct I2C * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = I2CBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;

  interface->callback = nullptr;
  interface->callbackArgument = nullptr;
  interface->address = 0;
  interface->rate = config->rate;
  interface->blocking = true;
  interface->active = false;
  interface->error = false;
  interface->reading = false;

  /* Rate should be initialized after block selection */
  i2cSetRate(&interface->base, config->rate);

  BL_I2C_Type * const reg = interface->base.reg;

  /* Clear all flags and FIFOs */
  reg->I2C_INT_STS = 0;
  reg->I2C_FIFO_CONFIG_0 = FIFO_CONFIG_0_TFICLR | FIFO_CONFIG_0_RFICLR;
  reg->I2C_CONFIG = CONFIG_SCLSEN;

#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Set interrupt priority and enable the interface */
  irqSetPriority(interface->base.irq, config->priority);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_BOUFFALO_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  struct I2C * const interface = object;
  BL_I2C_Type * const reg = interface->base.reg;

  /* Disable the interface */
  reg->I2C_INT_STS = 0;
  reg->I2C_CONFIG = 0;
  interface->active = false;

  irqDisable(interface->base.irq);

#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_PM
  pmUnregister(interface);
#endif

  I2CBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void i2cSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2C * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result i2cGetParam(void *object, int parameter, void *data)
{
  struct I2C * const interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      *(uint32_t *)data = interface->address;
      return E_OK;

#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_RC
    case IF_RATE:
      *(uint32_t *)data = i2cGetRate(&interface->base);
      return E_OK;
#endif

    case IF_STATUS:
      if (interface->error)
        return E_ERROR;
      if (interface->active)
        return E_BUSY;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result i2cSetParam(void *object, int parameter, const void *data)
{
  struct I2C * const interface = object;

  /* Additional I2C parameters */
  switch ((enum I2CParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_RECOVERY
    case IF_I2C_BUS_RECOVERY:
    {
      BL_I2C_Type * const reg = interface->base.reg;

      /* Disable the interface and clear the bus busy status */
      reg->I2C_CONFIG = 0;
      reg->I2C_BUS_BUSY = BUS_BUSY_BUSYCLR;

      i2cRecoverBus(&interface->base);

      /* Re-enable the master function */
      reg->I2C_CONFIG = CONFIG_SCLSEN;

      return E_OK;
    }
#endif

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      if (*(const uint32_t *)data <= 127)
      {
        interface->address = *(const uint32_t *)data;
        return E_OK;
      }
      else
        return E_VALUE;

#ifdef CONFIG_PLATFORM_BOUFFALO_I2C_RC
    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      i2cSetRate(&interface->base, interface->rate);
      return E_OK;
#endif

    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t i2cRead(void *object, void *buffer, size_t length)
{
  struct I2C * const interface = object;
  BL_I2C_Type * const reg = interface->base.reg;

  if (!length || length > I2C_PACKET_LENGTH_MAX)
    return 0;

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = length;
  interface->txLeft = 0;
  interface->error = false;
  interface->active = true;
  interface->reading = true;

  reg->I2C_CONFIG = CONFIG_PKTDIR | CONFIG_SLVADDR(interface->address)
      | CONFIG_PKTLEN(length - 1) | CONFIG_DEGCNT(4) | CONFIG_DEGEN;
  reg->I2C_FIFO_CONFIG_0 = FIFO_CONFIG_0_TFICLR | FIFO_CONFIG_0_RFICLR;
  /*
   * The NAK interrupt stays enabled, because a slave NAK on the
   * address phase must abort the read. The final NAK generated by the
   * master after the last data byte is not an error, the interrupt
   * handler recognizes it by the transfer direction and the empty
   * RX counter.
   */
  reg->I2C_INT_STS = INT_STS_RXFEN | INT_STS_ENDEN | INT_STS_NAKEN
      | INT_STS_ARBEN | INT_STS_FEREN;

  /* Start the transfer */
  reg->I2C_CONFIG |= CONFIG_MEN;
  irqEnable(interface->base.irq);

  if (interface->blocking)
  {
    /*
     * The interface is ready only after the stop condition has been
     * issued on the bus, therefore wait for the transfer end.
     */
    while (interface->active && !interface->error)
      barrier();

    if (interface->error)
      return 0;
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t i2cWrite(void *object, const void *buffer, size_t length)
{
  struct I2C * const interface = object;
  BL_I2C_Type * const reg = interface->base.reg;

  if (!length || length > I2C_PACKET_LENGTH_MAX)
    return 0;

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = 0;
  interface->txLeft = length;
  interface->error = false;
  interface->active = true;
  interface->reading = false;

  reg->I2C_CONFIG = CONFIG_SLVADDR(interface->address)
      | CONFIG_PKTLEN(length - 1) | CONFIG_DEGCNT(4) | CONFIG_DEGEN;
  reg->I2C_FIFO_CONFIG_0 = FIFO_CONFIG_0_TFICLR | FIFO_CONFIG_0_RFICLR;
  reg->I2C_INT_STS = INT_STS_TXFEN | INT_STS_ENDEN | INT_STS_NAKEN
      | INT_STS_ARBEN | INT_STS_FEREN;

  /* Start the transfer */
  reg->I2C_CONFIG |= CONFIG_MEN;
  irqEnable(interface->base.irq);

  if (interface->blocking)
  {
    /*
     * The interface is ready only after the stop condition has been
     * issued on the bus, therefore wait for the transfer end.
     */
    while (interface->active && !interface->error)
      barrier();

    if (interface->error)
      return 0;
  }

  return length;
}
