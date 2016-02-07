/*
 * i2c.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <memory.h>
#include <platform/nxp/gen_1/i2c_defs.h>
#include <platform/nxp/i2c.h>
/*----------------------------------------------------------------------------*/
enum state
{
  STATE_IDLE,
  STATE_ADDRESS,
  STATE_TRANSMIT,
  STATE_RECEIVE,
  STATE_ERROR
};
/*----------------------------------------------------------------------------*/
/* Master transmitter and receiver modes */
enum status
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
static enum result i2cInit(void *, const void *);
static void i2cDeinit(void *);
static enum result i2cCallback(void *, void (*)(void *), void *);
static enum result i2cGet(void *, enum ifOption, void *);
static enum result i2cSet(void *, enum ifOption, const void *);
static uint32_t i2cRead(void *, uint8_t *, uint32_t);
static uint32_t i2cWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass i2cTable = {
    .size = sizeof(struct I2c),
    .init = i2cInit,
    .deinit = i2cDeinit,

    .callback = i2cCallback,
    .get = i2cGet,
    .set = i2cSet,
    .read = i2cRead,
    .write = i2cWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const I2c = &i2cTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2c * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;
  bool event = false;

  switch ((enum status)reg->STAT)
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

    /* Data byte has been transmitted */
    case STATUS_DATA_TRANSMITTED_ACK:
    case STATUS_DATA_TRANSMITTED_NACK:
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
  reg->CONCLR = CONCLR_SIC;

  if (interface->state == STATE_IDLE || interface->state == STATE_ERROR)
    irqDisable(interface->base.irq);

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configBase)
{
  const struct I2cConfig * const config = configBase;
  const struct I2cBaseConfig baseConfig = {
      .channel = config->channel,
      .scl = config->scl,
      .sda = config->sda
  };
  struct I2c * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = I2cBase->init(object, &baseConfig)) != E_OK)
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
static void i2cDeinit(void *object)
{
  struct I2c * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  reg->CONCLR = CONCLR_I2ENC; /* Disable I2C interface */
  I2cBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result i2cCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2c * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2cGet(void *object, enum ifOption option, void *data)
{
  struct I2c * const interface = object;

  switch (option)
  {
    case IF_STATUS:
      if (!interface->blocking && interface->state == STATE_ERROR)
        return E_ERROR;
      else
        return interface->state != STATE_IDLE ? E_BUSY : E_OK;

    case IF_RATE:
      *(uint32_t *)data = i2cGetRate(object);
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result i2cSet(void *object, enum ifOption option, const void *data)
{
  struct I2c * const interface = object;

  /* Additional I2C options */
  switch ((enum i2cOption)option)
  {
    case IF_I2C_SENDSTOP:
      interface->sendStopBit = *(const uint32_t *)data ? true : false;
      return E_OK;

    default:
      break;
  }

  switch (option)
  {
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;

    case IF_ADDRESS:
      interface->address = *(const uint32_t *)data;
      return E_OK;

    case IF_RATE:
      i2cSetRate(object, *(const uint32_t *)data);
      return E_OK;

    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t i2cRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct I2c * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;

  interface->rxLeft = (uint16_t)length;
  interface->txLeft = 0;
  interface->rxBuffer = buffer;
  interface->txBuffer = 0;

  interface->state = STATE_ADDRESS;
  reg->CONSET = CONSET_STA;
  /*
   * Flag should be cleared when Repeated Start generation is enabled.
   * Interrupt with previous interface state will be generated otherwise.
   */
  if (!interface->sendStopBit)
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
static uint32_t i2cWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct I2c * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;

  interface->rxLeft = 0;
  interface->txLeft = (uint16_t)length;
  interface->rxBuffer = 0;
  interface->txBuffer = buffer;

  interface->state = STATE_ADDRESS;
  /*
   * Start condition generation can be delayed when the bus is not free.
   * After Stop condition being transmitted Start will be generated
   * automatically.
   */
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
