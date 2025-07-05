/*
 * i2c.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/lpc/gen_1/i2c_defs.h>
#include <halm/platform/lpc/i2c.h>
#include <halm/pm.h>
#include <assert.h>
#include <limits.h>
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_ADDRESS,
  STATE_TRANSMIT,
  STATE_RECEIVE,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);

#ifdef CONFIG_PLATFORM_LPC_I2C_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);
static void i2cSetCallback(void *, void (*)(void *), void *);
static enum Result i2cGetParam(void *, int, void *);
static enum Result i2cSetParam(void *, int, const void *);
static size_t i2cRead(void *, void *, size_t);
static size_t i2cWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_LPC_I2C_NO_DEINIT
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
  LPC_I2C_Type * const reg = interface->base.reg;
  const uint8_t stat = reg->STAT;
  bool clearInterrupt = true;
  bool event = false;

  switch (stat)
  {
    /* Start or Repeated Start conditions have been transmitted */
    case STATUS_START_TRANSMITTED:
    case STATUS_RESTART_TRANSMITTED:
      if (interface->state == STATE_ADDRESS)
      {
        reg->DAT = (interface->txLeft ? DATA_WRITE : DATA_READ)
            | (interface->address << 1);
      }
      reg->CONCLR = CONCLR_STAC;
      break;

    /* Slave address and write bit have been transmitted, ACK received */
    case STATUS_SLAVE_WRITE_ACK:
      reg->DAT = *(const uint8_t *)interface->buffer;
      ++interface->buffer;
      --interface->txLeft;

      interface->state = STATE_TRANSMIT;
      break;

    /* Slave address and read bit have been transmitted, ACK received */
    case STATUS_SLAVE_READ_ACK:
      if (interface->rxLeft > 1)
        reg->CONSET = CONSET_AA; /* Assert ACK after data is received */
      else
        reg->CONCLR = CONCLR_AAC; /* Assert NACK after data is received */

      interface->state = STATE_RECEIVE;
      break;

    /* Data byte has been transmitted, ACK received */
    case STATUS_MASTER_DATA_TRANSMITTED_ACK:
      if (interface->state != STATE_TRANSMIT)
        break;

      /* Send next byte or stop transmission */
      if (interface->txLeft)
      {
        reg->DAT = *(const uint8_t *)interface->buffer;
        ++interface->buffer;
        --interface->txLeft;
      }
      else
      {
        if (interface->sendRepeatedStart)
        {
          interface->sendRepeatedStart = false;
          clearInterrupt = false;
        }
        else
          reg->CONSET = CONSET_STO;

        interface->state = STATE_IDLE;
        event = true;
      }
      break;

    /* Data has been received and ACK has been returned */
    case STATUS_MASTER_DATA_RECEIVED_ACK:
      *(uint8_t *)interface->buffer = reg->DAT;
      ++interface->buffer;
      --interface->rxLeft;

      if (interface->rxLeft > 1)
        reg->CONSET = CONSET_AA;
      else
        reg->CONCLR = CONCLR_AAC;
      break;

    /* Last byte of data has been received and NACK has been returned */
    case STATUS_MASTER_DATA_RECEIVED_NACK:
      if (interface->state != STATE_RECEIVE)
        break;

      *(uint8_t *)interface->buffer = reg->DAT;
      ++interface->buffer;
      --interface->rxLeft;

      reg->CONSET = CONSET_STO;
      interface->state = STATE_IDLE;
      event = true;
      break;

    /* External interference disturbed the internal signals */
    case STATUS_BUS_ERROR:
    /* Arbitration has been lost during transmission or reception */
    case STATUS_ARBITRATION_LOST:
    /* Data byte has been transmitted, NACK received */
    case STATUS_MASTER_DATA_TRANSMITTED_NACK:
    /* Address and direction bit have not been acknowledged */
    case STATUS_SLAVE_WRITE_NACK:
    case STATUS_SLAVE_READ_NACK:
      interface->sendRepeatedStart = false;

      reg->CONSET = CONSET_STO;
      interface->state = STATE_ERROR;
      event = true;
      break;

    case STATUS_NO_INFORMATION:
      clearInterrupt = false;
      break;

    default:
      break;
  }

  if (clearInterrupt)
    reg->CONCLR = CONCLR_SIC;

  if (interface->state == STATE_IDLE || interface->state == STATE_ERROR)
    irqDisable(interface->base.irq);

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_LPC_I2C_PM
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

  interface->base.handler = interruptHandler;

  interface->address = 0;
  interface->callback = NULL;
  interface->blocking = true;
  interface->rate = config->rate;
  interface->sendRepeatedStart = false;
  interface->state = STATE_IDLE;

  /* Rate should be initialized after block selection */
  i2cSetRate(&interface->base, config->rate);

  LPC_I2C_Type * const reg = interface->base.reg;

  /* Clear all flags */
  reg->CONCLR = CONCLR_AAC | CONCLR_SIC | CONCLR_STAC | CONCLR_I2ENC;

#ifdef CONFIG_PLATFORM_LPC_I2C_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the interface */
  reg->CONSET = CONSET_I2EN;

  irqSetPriority(interface->base.irq, config->priority);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  struct I2C * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  /* Disable the interface */
  reg->CONCLR = CONCLR_I2ENC;

#ifdef CONFIG_PLATFORM_LPC_I2C_PM
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

#ifndef CONFIG_PLATFORM_LPC_I2C_RC
  (void)data;
#endif

  switch ((enum IfParameter)parameter)
  {
#ifdef CONFIG_PLATFORM_LPC_I2C_RC
    case IF_RATE:
      *(uint32_t *)data = i2cGetRate(object);
      return E_OK;
#endif

    case IF_STATUS:
      if (interface->state != STATE_ERROR)
        return interface->state != STATE_IDLE ? E_BUSY : E_OK;
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

#ifdef CONFIG_PLATFORM_LPC_I2C_RECOVERY
    case IF_I2C_BUS_RECOVERY:
    {
      LPC_I2C_Type * const reg = interface->base.reg;

      reg->CONSET = CONSET_STO;
      i2cRecoverBus(&interface->base);
      i2cConfigPins(&interface->base);
      interface->sendRepeatedStart = false;

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

#ifdef CONFIG_PLATFORM_LPC_I2C_RC
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
  LPC_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = length;
  interface->txLeft = 0;

  interface->state = STATE_ADDRESS;
  reg->CONSET = CONSET_STA;

  /* Continue previous transmission when Repeated Start is enabled */
  if (reg->STAT == STATUS_MASTER_DATA_TRANSMITTED_ACK)
    reg->CONCLR = CONCLR_SIC;

  irqEnable(interface->base.irq);

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t i2cWrite(void *object, const void *buffer, size_t length)
{
  struct I2C * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = 0;
  interface->txLeft = length;

  /* Stop previous transmission when Repeated Start is enabled */
  if (reg->STAT == STATUS_MASTER_DATA_TRANSMITTED_ACK)
  {
    reg->CONSET = CONSET_STO;
    reg->CONCLR = CONCLR_SIC;
  }

  /*
   * Start condition generation can be delayed when the bus is not free.
   * After Stop condition being transmitted Start will be generated
   * automatically.
   */
  interface->state = STATE_ADDRESS;
  reg->CONSET = CONSET_STA;

  irqEnable(interface->base.irq);

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return length;
}
