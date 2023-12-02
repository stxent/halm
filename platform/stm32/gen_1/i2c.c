/*
 * i2c.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/stm32/i2c.h>
#include <halm/platform/stm32/gen_1/i2c_defs.h>
#include <halm/pm.h>
#include <assert.h>
#include <limits.h>
/*----------------------------------------------------------------------------*/
enum Status
{
  STATUS_OK,
  STATUS_START,
  STATUS_TRANSMIT,
  STATUS_RECEIVE,
  STATUS_ERROR
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct I2C *, uint8_t, uint8_t);
static void interruptHandler(void *);
static void reconfigure(struct I2C *);
static void rxDmaHandler(void *);
static void txDmaHandler(void *);

#ifdef CONFIG_PLATFORM_STM32_I2C_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);
static void i2cSetCallback(void *, void (*)(void *), void *);
static enum Result i2cGetParam(void *, int, void *);
static enum Result i2cSetParam(void *, int, const void *);
static size_t i2cRead(void *, void *, size_t);
static size_t i2cWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_STM32_I2C_NO_DEINIT
static void i2cDeinit(void *);
#else
#define i2cDeinit deletedDestructorTrap
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
static bool dmaSetup(struct I2C *interface, uint8_t rxStream, uint8_t txStream)
{
  static const struct DmaSettings rxDmaSettings = {
      .source = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .increment = false
      },
      .destination = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_BYTE,
          .increment = true
      }
  };
  static const struct DmaSettings txDmaSettings = {
      .source = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_BYTE,
          .increment = true
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_BYTE,
          .increment = false
      }
  };

  interface->rxDma = i2cMakeOneShotDma(
      interface->base.channel,
      rxStream,
      DMA_PRIORITY_LOW,
      DMA_TYPE_P2M
  );
  if (interface->rxDma == NULL)
    return false;
  dmaConfigure(interface->rxDma, &rxDmaSettings);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = i2cMakeOneShotDma(
      interface->base.channel,
      txStream,
      DMA_PRIORITY_LOW,
      DMA_TYPE_M2P
  );
  if (interface->txDma == NULL)
    return false;
  dmaConfigure(interface->txDma, &txDmaSettings);
  dmaSetCallback(interface->txDma, txDmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  static const uint16_t ERROR_MASK = SR1_BERR | SR1_ARLO | SR1_AF;

  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;
  const uint16_t status = reg->SR1;
  bool error = (status & ERROR_MASK) != 0;
  bool event = false;

  reg->SR1 = 0;

  if (!error)
  {
    if (status & SR1_SB)
    {
      if (interface->txLeft)
      {
        reg->CR1 &= ~CR1_ACK;
        reg->CR2 |= CR2_LAST;
        reg->DR = (interface->address << 1) | DR_WRITE;

        dmaAppend(interface->txDma, (void *)&reg->DR,
            (const void *)interface->buffer, interface->txLeft);

        if (dmaEnable(interface->txDma) != E_OK)
        {
          interface->status = STATUS_ERROR;
          error = true;
        }
        else
          interface->status = STATUS_TRANSMIT;
      }
      else
      {
        if (interface->rxLeft == 1)
          reg->CR1 &= ~CR1_ACK;
        else
          reg->CR1 |= CR1_ACK;
        reg->CR2 |= CR2_LAST;
        reg->DR = (interface->address << 1) | DR_READ;

        dmaAppend(interface->rxDma, (void *)interface->buffer,
            (const void *)&reg->DR, interface->rxLeft);

        if (dmaEnable(interface->rxDma) != E_OK)
        {
          interface->status = STATUS_ERROR;
          error = true;
        }
        else
          interface->status = STATUS_RECEIVE;
      }

      if (!error)
        reg->CR2 |= CR2_DMAEN;
    }
    else if (status & SR1_ADDR)
    {
      /* Read SR2 to clear ADDR flag */
      if (!(reg->SR2 & SR2_MSL))
        error = true;
    }
    else if (status & SR1_BTF)
    {
      if (interface->dataTransmitted)
      {
        if (!interface->sendRepeatedStart)
          reg->CR1 |= CR1_STOP;

        interface->dataTransmitted = false;
        interface->sendRepeatedStart = false;
        interface->status = STATUS_OK;
        event = true;
      }
      else
      {
        /* Clear BTF flag */
        (void)reg->DR;
      }
    }
  }

  if (error)
  {
    if (interface->status == STATUS_RECEIVE)
    {
      reg->CR2 &= ~CR2_DMAEN;
      dmaDisable(interface->rxDma);
    }
    if (interface->status == STATUS_TRANSMIT)
    {
      reg->CR2 &= ~CR2_DMAEN;
      dmaDisable(interface->txDma);
    }

    reg->CR1 |= CR1_STOP;

    interface->sendRepeatedStart = false;
    interface->status = STATUS_ERROR;
    event = true;
  }

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void reconfigure(struct I2C *interface)
{
  STM_I2C_Type * const reg = interface->base.reg;

  interface->dataTransmitted = false;
  interface->sendRepeatedStart = false;
  interface->status = STATUS_OK;

  /* Exit reset mode */
  reg->CR1 = 0;

  /* Configure interface mode and rate */
  i2cSetRate(&interface->base, interface->rate);
  /* Enable interrupts */
  reg->CR2 = CR2_ITERREN | CR2_ITEVTEN;

  /* Enable the interface */
  reg->CR1 |= CR1_PE;
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;

  reg->CR2 &= ~CR2_DMAEN;
  reg->CR1 |= CR1_STOP;

  interface->status = dmaStatus(interface->rxDma) == E_OK ?
      STATUS_OK : STATUS_ERROR;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;

  reg->CR2 &= ~CR2_DMAEN;

  if (dmaStatus(interface->txDma) != E_OK)
  {
    reg->CR1 |= CR1_STOP;

    interface->sendRepeatedStart = false;
    interface->status = STATUS_ERROR;

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
  else
    interface->dataTransmitted = true;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_I2C_PM
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
  assert(config != NULL);

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

  if (!dmaSetup(interface, config->rxDma, config->txDma))
    return E_ERROR;

  interface->base.handler = interruptHandler;

  interface->callback = NULL;
  interface->rate = config->rate;
  interface->address = 0;
  interface->blocking = true;

  STM_I2C_Type * const reg = interface->base.reg;

  /* Reset the interface */
  reg->CR1 = CR1_SWRST;
  /* Configure and enable the interface */
  reconfigure(interface);

#ifdef CONFIG_PLATFORM_STM32_I2C_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  if (interface->base.irq.er != IRQ_RESERVED)
  {
    irqSetPriority(interface->base.irq.er, config->priority);
    irqClearPending(interface->base.irq.er);
    irqEnable(interface->base.irq.er);
  }
  irqSetPriority(interface->base.irq.ev, config->priority);
  irqClearPending(interface->base.irq.ev);
  irqEnable(interface->base.irq.ev);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_STM32_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;

  /* Disable the interface */
  reg->CR1 = 0;

  /* Disable interrupts */
  irqDisable(interface->base.irq.ev);
  if (interface->base.irq.er != IRQ_RESERVED)
    irqDisable(interface->base.irq.er);
  reg->CR2 = 0;

#ifdef CONFIG_PLATFORM_STM32_I2C_PM
  pmUnregister(interface);
#endif

  /* Free DMA channels */
  deinit(interface->txDma);
  deinit(interface->rxDma);

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
    case IF_STATUS:
      if (interface->status != STATUS_ERROR)
        return interface->status != STATUS_OK ? E_BUSY : E_OK;
      else
        return E_ERROR;

    case IF_RATE:
      *(uint32_t *)data = i2cGetRate(object);
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
    case IF_I2C_REPEATED_START:
      interface->sendRepeatedStart = true;
      return E_OK;

    case IF_I2C_BUS_RECOVERY:
    {
      STM_I2C_Type * const reg = interface->base.reg;

      if (interface->status == STATUS_RECEIVE)
      {
        reg->CR2 &= ~CR2_DMAEN;
        dmaDisable(interface->rxDma);
      }
      if (interface->status == STATUS_TRANSMIT)
      {
        reg->CR2 &= ~CR2_DMAEN;
        dmaDisable(interface->txDma);
      }

      /* Modified sequence from the Errata Sheet ES096 */

      /* Enable software reset */
      reg->CR1 = CR1_SWRST;

      i2cRecoverBus(&interface->base);
      i2cConfigPins(&interface->base);

      reconfigure(interface);
      return E_OK;
    }

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

    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_RATE:
      interface->rate = *(const uint32_t *)data;
      i2cSetRate(&interface->base, interface->rate);
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
  STM_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;
  if (length > USHRT_MAX)
    length = USHRT_MAX;

  dmaDisable(interface->rxDma);
  dmaDisable(interface->txDma);

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = length;
  interface->txLeft = 0;
  interface->status = STATUS_START;
  interface->dataTransmitted = false;

  reg->SR1 = 0;
  reg->CR1 |= CR1_START;

  if (interface->blocking)
  {
    while (interface->status != STATUS_OK && interface->status != STATUS_ERROR)
      barrier();

    if (interface->status == STATUS_ERROR)
      return 0;
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t i2cWrite(void *object, const void *buffer, size_t length)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;
  if (length > USHRT_MAX)
    length = USHRT_MAX;

  dmaDisable(interface->rxDma);
  dmaDisable(interface->txDma);

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = 0;
  interface->txLeft = length;
  interface->status = STATUS_START;
  interface->dataTransmitted = false;

  reg->SR1 = 0;
  reg->CR1 |= CR1_START;

  if (interface->blocking)
  {
    while (interface->status != STATUS_OK && interface->status != STATUS_ERROR)
      barrier();

    if (interface->status == STATUS_ERROR)
      return 0;
  }

  return length;
}
