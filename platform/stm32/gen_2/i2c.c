/*
 * i2c.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/stm32/i2c.h>
#include <halm/platform/stm32/gen_2/i2c_defs.h>
#include <halm/pm.h>
#include <assert.h>
#include <limits.h>
/*----------------------------------------------------------------------------*/
#define ADDRESS_TYPE_MASK       MASK(15)
#define ADDRESS_TYPE_7BIT_MASK  MASK(7)
#define ADDRESS_TYPE_7BIT       0
#define ADDRESS_TYPE_10BIT_MASK MASK(10)
#define ADDRESS_TYPE_10BIT      BIT(15)

enum Status
{
  STATUS_OK,
  STATUS_TRANSMIT,
  STATUS_RECEIVE,
  STATUS_ERROR
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct I2C *, uint8_t, uint8_t);
static uint32_t insertDeviceAddress(const struct I2C *, uint32_t);
static void interruptHandler(void *);
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
      DMA_PRIORITY_MEDIUM,
      DMA_TYPE_P2M
  );
  if (interface->rxDma == NULL)
    return false;
  dmaConfigure(interface->rxDma, &rxDmaSettings);
  dmaSetCallback(interface->rxDma, rxDmaHandler, interface);

  interface->txDma = i2cMakeOneShotDma(
      interface->base.channel,
      txStream,
      DMA_PRIORITY_MEDIUM,
      DMA_TYPE_M2P
  );
  if (interface->txDma == NULL)
    return false;
  dmaConfigure(interface->txDma, &txDmaSettings);
  dmaSetCallback(interface->txDma, txDmaHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static uint32_t insertDeviceAddress(const struct I2C *interface, uint32_t cr2)
{
  cr2 &= ~(CR2_SADD10_MASK | CR2_ADD10 | CR2_HEAD10R);

  if (interface->address & ADDRESS_TYPE_10BIT)
  {
    cr2 = CR2_SADD10(interface->address & ADDRESS_TYPE_10BIT_MASK)
        | CR2_ADD10 | CR2_HEAD10R;
  }
  else
    cr2 = CR2_SADD7(interface->address & ADDRESS_TYPE_7BIT_MASK);

  return cr2;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;
  const uint32_t status = reg->ISR;
  bool event = false;

  reg->ICR = ICR_NACKCF | ICR_STOPCF | ICR_BERRCF | ICR_ARLOCF;

  if (status & ISR_TCR)
  {
    uint32_t cr2 = reg->CR2 & ~(CR2_NBYTES_MASK | CR2_RELOAD);

    interface->left -= MIN(interface->left, CR2_NBYTES_MAX);

    cr2 |= CR2_NBYTES(MIN(interface->left, CR2_NBYTES_MAX));
    if (interface->left > CR2_NBYTES_MAX)
      cr2 |= CR2_RELOAD;

    /* Transfer a next part of data */
    reg->CR2 = cr2;
  }
  else if (status & (ISR_NACKF | ISR_BERR | ISR_ARLO))
  {
    interface->status = STATUS_ERROR;
    event = true;
  }
  else
  {
    interface->status = STATUS_OK;
    event = true;
  }

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void rxDmaHandler(void *object)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;

  interface->status = dmaStatus(interface->rxDma) == E_OK ?
      STATUS_OK : STATUS_ERROR;

  if (interface->status == STATUS_ERROR)
    reg->CR2 |= CR2_STOP;
}
/*----------------------------------------------------------------------------*/
static void txDmaHandler(void *object)
{
  struct I2C * const interface = object;
  STM_I2C_Type * const reg = interface->base.reg;

  interface->status = dmaStatus(interface->txDma) == E_OK ?
      STATUS_OK : STATUS_ERROR;
  interface->sendRepeatedStart = false;

  if (interface->status == STATUS_ERROR)
    reg->CR2 |= CR2_STOP;
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

  interface->address = 0;
  interface->callback = NULL;
  interface->blocking = true;
  interface->rate = config->rate;
  interface->sendRepeatedStart = false;
  interface->status = STATUS_OK;

  STM_I2C_Type * const reg = interface->base.reg;

  /* Clear interrupt flags */
  reg->ICR = ICR_NACKCF | ICR_STOPCF | ICR_BERRCF | ICR_ARLOCF;
  /* Enable Analog Filter, Clock Stretching, DMA and interrupts */
  reg->CR1 = CR1_NACKIE | CR1_STOPIE | CR1_TCIE | CR1_ERRIE
      | CR1_TXDMAEN | CR1_RXDMAEN;

  /* Rate should be initialized before interface enabling */
  i2cSetRate(&interface->base, config->rate);

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

  /* Enable the interface */
  reg->CR1 |= CR1_PE;

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

#ifndef CONFIG_PLATFORM_STM32_I2C_RC
  (void)data;
#endif

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_STM32_I2C_RC
    case IF_RATE:
      *(uint32_t *)data = i2cGetRate(object);
      return E_OK;
#endif

    case IF_STATUS:
      if (interface->status != STATUS_ERROR)
        return interface->status != STATUS_OK ? E_BUSY : E_OK;
      else
        return E_ERROR;

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

#ifdef CONFIG_PLATFORM_STM32_I2C_RECOVERY
    case IF_I2C_BUS_RECOVERY:
    {
      STM_I2C_Type * const reg = interface->base.reg;

      reg->CR1 &= ~CR1_PE;
      i2cRecoverBus(&interface->base);
      i2cConfigPins(&interface->base);
      reg->CR1 |= CR1_PE;

      interface->sendRepeatedStart = false;
      interface->status = STATUS_OK;
      return E_OK;
    }
#endif

    case IF_I2C_10BIT_ADDRESS:
      if (*(const uint8_t *)data)
        interface->address |= ADDRESS_TYPE_10BIT;
      else
        interface->address &= ~ADDRESS_TYPE_10BIT;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      if (*(const uint32_t *)data <= 1023)
      {
        interface->address = (interface->address & ADDRESS_TYPE_10BIT)
            | *(const uint32_t *)data;
        return E_OK;
      }
      else
        return E_VALUE;

#ifdef CONFIG_PLATFORM_STM32_I2C_RC
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
  STM_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;
  if (length > DMA_MAX_TRANSFER_SIZE)
    length = DMA_MAX_TRANSFER_SIZE;

  dmaDisable(interface->rxDma);
  dmaDisable(interface->txDma);

  dmaAppend(interface->rxDma, buffer, (const void *)&reg->RXDR, length);
  if (dmaEnable(interface->rxDma) != E_OK)
  {
    interface->status = STATUS_ERROR;
    return 0;
  }

  interface->left = length;
  interface->status = STATUS_RECEIVE;

  uint32_t cr2 = reg->CR2;

  cr2 &= ~(CR2_NBYTES_MASK | CR2_RELOAD);
  cr2 = insertDeviceAddress(interface, cr2);

  cr2 |= CR2_NBYTES(MIN(length, CR2_NBYTES_MAX)) | CR2_RD_WRN | CR2_AUTOEND;
  if (length > CR2_NBYTES_MAX)
    cr2 |= CR2_RELOAD;

  reg->CR2 = cr2 | CR2_START;

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
  if (length > DMA_MAX_TRANSFER_SIZE)
    length = DMA_MAX_TRANSFER_SIZE;

  dmaDisable(interface->rxDma);
  dmaDisable(interface->txDma);

  dmaAppend(interface->txDma, (void *)&reg->TXDR, buffer, length);
  if (dmaEnable(interface->txDma) != E_OK)
  {
    interface->status = STATUS_ERROR;
    return 0;
  }

  interface->left = length;
  interface->status = STATUS_TRANSMIT;

  uint32_t cr2 = reg->CR2;

  cr2 &= ~(CR2_RD_WRN | CR2_NBYTES_MASK | CR2_RELOAD | CR2_AUTOEND);
  cr2 = insertDeviceAddress(interface, cr2);

  cr2 |= CR2_NBYTES(MIN(length, CR2_NBYTES_MAX));
  if (length > CR2_NBYTES_MAX)
    cr2 |= CR2_RELOAD;
  if (!interface->sendRepeatedStart)
    cr2 |= CR2_AUTOEND;

  reg->CR2 = cr2 | CR2_START;

  if (interface->blocking)
  {
    while (interface->status != STATUS_OK && interface->status != STATUS_ERROR)
      barrier();

    if (interface->status == STATUS_ERROR)
      return 0;
  }

  return length;
}
