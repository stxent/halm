/*
 * i2c.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/i2c.h>
#include <platform/nxp/i2c_defs.h>
/*----------------------------------------------------------------------------*/
enum status
{
  /* Implemented only master transmitter and receiver modes */
  STAT_START_TRANSMITTED      = 0x08, /* Start condition transmitted */
  STAT_RESTART_TRANSMITTED    = 0x10, /* Repeated start condition transmitted */
  STAT_SLAVE_WRITE_ACK        = 0x18,
  STAT_SLAVE_WRITE_NACK       = 0x20,
  STAT_DATA_TRANSMITTED_ACK   = 0x28,
  STAT_DATA_TRANSMITTED_NACK  = 0x30,
  STAT_ARBITRATION_LOST       = 0x38,
  STAT_SLAVE_READ_ACK         = 0x40,
  STAT_SLAVE_READ_NACK        = 0x48,
  STAT_DATA_RECEIVED_ACK      = 0x50,
  STAT_DATA_RECEIVED_NACK     = 0x58
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
const struct InterfaceClass *I2c = &i2cTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2c *interface = object;
  LPC_I2C_Type *reg = interface->parent.reg;
  bool event = false;

  switch (reg->STAT)
  {
    /* Start or repeated start condition has been transmitted */
    case STAT_START_TRANSMITTED:
    case STAT_RESTART_TRANSMITTED:
      if (interface->txCount)
        reg->DAT = interface->address | DATA_WRITE;
      else
        reg->DAT = interface->address | DATA_READ;
      reg->CONCLR = CONCLR_STAC;
      break;
    /* Slave address and write bit have been transmitted, ACK received */
    case STAT_SLAVE_WRITE_ACK:
      --interface->txCount;
      reg->DAT = *interface->txBuffer++;
      interface->state = I2C_TRANSMIT;
      break;
    /* Slave address and read bit have been transmitted, ACK received */
    case STAT_SLAVE_READ_ACK:
      if (interface->rxCount == 1)
        reg->CONCLR = CONCLR_AAC; /* Assert NACK after data is received */
      else
        reg->CONSET = CONSET_AA; /* Assert ACK after data is received */
      interface->state = I2C_RECEIVE;
      break;
    /* Data byte has been transmitted */
    case STAT_DATA_TRANSMITTED_ACK:
    case STAT_DATA_TRANSMITTED_NACK:
      /* Send next byte or stop transmission */
      if (interface->txCount)
      {
        reg->DAT = *interface->txBuffer++;
        --interface->txCount;
      }
      else
      {
        if (interface->stop)
          reg->CONSET = CONSET_STO;
        interface->state = I2C_IDLE;
        event = true;
      }
      break;
    /* Data has been received and ACK has been returned */
    case STAT_DATA_RECEIVED_ACK:
      --interface->rxCount;
      *interface->rxBuffer++ = reg->DAT;
      if (interface->rxCount == 1)
        reg->CONCLR = CONCLR_AAC;
      else
        reg->CONSET = CONSET_AA;
      break;
    /* Last byte of data has been received and NACK has been returned */
    case STAT_DATA_RECEIVED_NACK:
      --interface->rxCount;
      *interface->rxBuffer++ = reg->DAT;
      reg->CONSET = CONSET_STO;
      interface->state = I2C_IDLE;
      event = true;
      break;
    /* Arbitration has been lost during transmission or reception */
    case STAT_ARBITRATION_LOST:
    /* Address and direction bit have not been acknowledged */
    case STAT_SLAVE_WRITE_NACK:
    case STAT_SLAVE_READ_NACK:
      reg->CONSET = CONSET_STO;
      interface->state = I2C_ERROR;
      event = true;
      break;
    default:
      break;
  }
  reg->CONCLR = CONCLR_SIC;

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configPtr)
{
  const struct I2cConfig * const config = configPtr;
  const struct I2cBaseConfig parentConfig = {
      .channel = config->channel,
      .scl = config->scl,
      .sda = config->sda
  };
  struct I2c *interface = object;
  enum result res;

  /* Call SSP class constructor */
  if ((res = I2cBase->init(object, &parentConfig)) != E_OK)
    return res;

  interface->address = 0;
  interface->callback = 0;
  interface->blocking = true;
  interface->lock = SPIN_UNLOCKED;
  interface->stop = true;
  interface->state = I2C_IDLE;

  /* Set pointer to interrupt handler */
  interface->parent.handler = interruptHandler;

  /* Rate should be initialized after block selection */
  i2cSetRate(object, config->rate);

  LPC_I2C_Type *reg = interface->parent.reg;

  /* Clear all flags */
  reg->CONCLR = CONCLR_AAC | CONCLR_SIC | CONCLR_STAC | CONCLR_I2ENC;
  /* Enable I2C interface */
  reg->CONSET = CONSET_I2EN;

  /* Set interrupt priority, lowest by default */
  irqSetPriority(interface->parent.irq, config->priority);
  /* Enable interrupt */
  irqEnable(interface->parent.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  struct I2c *interface = object;
  LPC_I2C_Type *reg = interface->parent.reg;

  irqDisable(interface->parent.irq);
  reg->CONCLR = CONCLR_I2ENC; /* Disable I2C interface */
  I2cBase->deinit(interface); /* Call I2C base class destructor */
}
/*----------------------------------------------------------------------------*/
static enum result i2cCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2c *interface = object;

  interface->callback = callback;
  interface->callbackArgument = argument;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2cGet(void *object, enum ifOption option, void *data)
{
  struct I2c *interface = object;

  switch (option)
  {
    case IF_STATUS:
      if (!interface->blocking && interface->state == I2C_ERROR)
        return E_ERROR;
      else
        return interface->state != I2C_IDLE ? E_BUSY : E_OK;
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
  struct I2c *interface = object;

  switch (option)
  {
    case IF_ACQUIRE:
      return spinTryLock(&interface->lock) ? E_OK : E_BUSY;
    case IF_BLOCKING:
      interface->blocking = true;
      return E_OK;
    case IF_DEVICE:
      interface->address = *(uint32_t *)data;
      return E_OK;
    case IF_RATE:
      i2cSetRate(object, *(uint32_t *)data);
      return E_OK;
    case IF_RELEASE:
      spinUnlock(&interface->lock);
      return E_OK;
    case IF_ZEROCOPY:
      interface->blocking = false;
      return E_OK;
    /* Additional I2C options */
    case IF_I2C_SENDSTOP:
      interface->stop = *(uint32_t *)data ? true : false;
      return E_OK;
    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t i2cRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct I2c *interface = object;
  LPC_I2C_Type *reg = interface->parent.reg;

  /* TODO Add interface recovery */
  /* TODO Check whether it is safe to wait for stop condition in I2C */
  while (reg->CONSET & CONSET_STO);

  interface->rxCount = (uint16_t)length;
  interface->txCount = 0;
  interface->rxBuffer = buffer;

  interface->state = I2C_ADDRESS;
  reg->CONSET = CONSET_STA;

  if (interface->blocking)
  {
    while (interface->state != I2C_IDLE && interface->state != I2C_ERROR);

    if (interface->state == I2C_ERROR)
      return 0;
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t i2cWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct I2c *interface = object;
  LPC_I2C_Type *reg = interface->parent.reg;

  while (reg->CONSET & CONSET_STO);

  interface->rxCount = 0;
  interface->txCount = (uint16_t)length;
  interface->txBuffer = buffer;

  interface->state = I2C_ADDRESS;
  reg->CONSET = CONSET_STA;

  if (interface->blocking)
  {
    while (interface->state != I2C_IDLE && interface->state != I2C_ERROR);

    if (interface->state == I2C_ERROR)
      return 0;
  }

  return length;
}
