/*
 * i2c.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/i2c_defs.h>
#include <halm/platform/lpc/i2c.h>
#include <xcore/memory.h>
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
/* Master transmitter and receiver modes */
enum Status
{
  /* Start condition transmitted */
  STATUS_START_TRANSMITTED      = 0x08,
  /* Repeated start condition transmitted */
  STATUS_RESTART_TRANSMITTED    = 0x10,
  STATUS_SLAVE_WRITE_ACK        = 0x18,
  STATUS_SLAVE_WRITE_NACK       = 0x20,
  STATUS_DATA_TRANSMITTED_ACK   = 0x28,
  STATUS_DATA_TRANSMITTED_NACK  = 0x30,
  STATUS_ARBITRATION_LOST       = 0x38,
  STATUS_SLAVE_READ_ACK         = 0x40,
  STATUS_SLAVE_READ_NACK        = 0x48,
  STATUS_DATA_RECEIVED_ACK      = 0x50,
  STATUS_DATA_RECEIVED_NACK     = 0x58
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
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
static void interruptHandler(void *object)
{
  struct I2C * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;
  bool clearInterrupt = true;
  bool event = false;

  switch (reg->STAT)
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
      --interface->txLeft;
      reg->DAT = *interface->txBuffer++;
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
    case STATUS_DATA_TRANSMITTED_ACK:
      if (interface->state != STATE_TRANSMIT)
        break;

      /* Send next byte or stop transmission */
      if (interface->txLeft)
      {
        reg->DAT = *interface->txBuffer++;
        --interface->txLeft;
      }
      else
      {
        if (interface->sendStopBit)
          reg->CONSET = CONSET_STO;
        else
          clearInterrupt = false;

        interface->state = STATE_IDLE;
        event = true;
      }
      break;

    /* Data has been received and ACK has been returned */
    case STATUS_DATA_RECEIVED_ACK:
      --interface->rxLeft;
      *interface->rxBuffer++ = reg->DAT;

      if (interface->rxLeft > 1)
        reg->CONSET = CONSET_AA;
      else
        reg->CONCLR = CONCLR_AAC;
      break;

    /* Last byte of data has been received and NACK has been returned */
    case STATUS_DATA_RECEIVED_NACK:
      if (interface->state != STATE_RECEIVE)
        break;

      --interface->rxLeft;
      *interface->rxBuffer++ = reg->DAT;

      reg->CONSET = CONSET_STO;
      interface->state = STATE_IDLE;
      event = true;
      break;

    /* Arbitration has been lost during transmission or reception */
    case STATUS_ARBITRATION_LOST:
    /* Data byte has been transmitted, NACK received */
    case STATUS_DATA_TRANSMITTED_NACK:
    /* Address and direction bit have not been acknowledged */
    case STATUS_SLAVE_WRITE_NACK:
    case STATUS_SLAVE_READ_NACK:
      reg->CONSET = CONSET_STO;
      interface->state = STATE_ERROR;
      event = true;
      break;

    default:
      break;
  }

  if (clearInterrupt)
    reg->CONCLR = CONCLR_SIC;

  if (interface->state == STATE_IDLE || interface->state == STATE_ERROR)
    irqDisable(interface->base.irq);

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CConfig * const config = configBase;
  assert(config);

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
  interface->callback = 0;
  interface->blocking = true;
  interface->sendStopBit = true;
  interface->state = STATE_IDLE;

  /* Rate should be initialized after block selection */
  i2cSetRate(object, config->rate);

  LPC_I2C_Type * const reg = interface->base.reg;

  /* Clear all flags */
  reg->CONCLR = CONCLR_AAC | CONCLR_SIC | CONCLR_STAC | CONCLR_I2ENC;
  /* Enable I2C interface */
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

  reg->CONCLR = CONCLR_I2ENC; /* Disable I2C interface */
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
      if (interface->blocking || interface->state != STATE_ERROR)
        return interface->state != STATE_IDLE ? E_BUSY : E_OK;
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
    case IF_I2C_SENDSTOP:
      interface->sendStopBit = *(const bool *)data;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      if (*(const uint16_t *)data <= 127)
      {
        interface->address = *(const uint16_t *)data;
        return E_OK;
      }
      else
        return E_VALUE;

    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_RATE:
      i2cSetRate(object, *(const uint32_t *)data);
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
  if (length > USHRT_MAX)
    length = USHRT_MAX;

  interface->rxLeft = length;
  interface->txLeft = 0;
  interface->rxBuffer = buffer;
  interface->txBuffer = 0;

  interface->state = STATE_ADDRESS;
  reg->CONSET = CONSET_STA;

  /* Continue previous transmission when Repeated Start is enabled */
  if (reg->STAT == STATUS_DATA_TRANSMITTED_ACK)
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
  if (length > USHRT_MAX)
    length = USHRT_MAX;

  interface->rxLeft = 0;
  interface->txLeft = length;
  interface->rxBuffer = 0;
  interface->txBuffer = buffer;

  /* Stop previous transmission when Repeated Start is enabled */
  if (reg->STAT == STATUS_DATA_TRANSMITTED_ACK)
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
