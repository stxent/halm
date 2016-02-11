/*
 * i2c_slave.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <platform/nxp/gen_1/i2c_defs.h>
#include <platform/nxp/i2c_slave.h>
#include <pm.h>
/*----------------------------------------------------------------------------*/
enum state
{
  STATE_IDLE,
  STATE_ADDRESS,
  STATE_DATA
};
/*----------------------------------------------------------------------------*/
/* Slave receiver and transmitter modes except general call modes */
enum status
{
  STATUS_ADDRESS_WRITE_RECEIVED = 0x60,
  STATUS_DATA_RECEIVED_ACK      = 0x80,
  STATUS_DATA_RECEIVED_NACK     = 0x88,
  STATUS_STOP_RECEIVED          = 0xA0,
  STATUS_ADDRESS_READ_RECEIVED  = 0xA8,
  STATUS_DATA_TRANSMITTED_ACK   = 0xB8,
  STATUS_DATA_TRANSMITTED_NACK  = 0xC0,
  STATUS_LAST_TRANSMITTED_ACK   = 0xC8
};
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_I2C_PM
static enum result powerStateHandler(void *, enum pmState);
#endif
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *, const void *);
static void i2cDeinit(void *);
static enum result i2cCallback(void *, void (*)(void *), void *);
static enum result i2cGet(void *, enum ifOption, void *);
static enum result i2cSet(void *, enum ifOption, const void *);
static size_t i2cRead(void *, void *, size_t);
static size_t i2cWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass i2cTable = {
    .size = sizeof(struct I2cSlave),
    .init = i2cInit,
    .deinit = i2cDeinit,

    .callback = i2cCallback,
    .get = i2cGet,
    .set = i2cSet,
    .read = i2cRead,
    .write = i2cWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const I2cSlave = &i2cTable;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct I2cSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;
  bool event = false;

  switch (reg->STAT)
  {
    case STATUS_ADDRESS_WRITE_RECEIVED:
      interface->state = STATE_ADDRESS;
      break;

    case STATUS_DATA_RECEIVED_ACK:
      if (interface->state == STATE_ADDRESS)
      {
        interface->external = reg->DAT;
        interface->state = STATE_DATA;
      }
      else
      {
        interface->cache[interface->external] = reg->DAT;

        if (++interface->external >= interface->size)
          interface->external = 0;
      }
      break;

    case STATUS_ADDRESS_READ_RECEIVED:
      interface->state = STATE_DATA;
    case STATUS_DATA_TRANSMITTED_ACK:
      reg->DAT = interface->cache[interface->external];

      /* Wrap current external position after reaching the end of cache */
      if (++interface->external >= interface->size)
        interface->external = 0;
      break;

    case STATUS_DATA_RECEIVED_NACK:
    case STATUS_STOP_RECEIVED:
    case STATUS_DATA_TRANSMITTED_NACK:
    case STATUS_LAST_TRANSMITTED_ACK:
      interface->state = STATE_IDLE;
      event = true;
      break;

    default:
      break;
  }

  reg->CONSET = CONSET_AA;
  reg->CONCLR = CONCLR_SIC;

  if (interface->callback && event)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_I2C_PM
static enum result powerStateHandler(void *object, enum pmState state)
{
  struct I2cSlave * const interface = object;

  if ((state == PM_SLEEP || state == PM_SUSPEND)
      && interface->state != STATE_IDLE)
  {
    return E_BUSY;
  }

  return E_OK;
}
#endif
/*----------------------------------------------------------------------------*/
static enum result i2cInit(void *object, const void *configBase)
{
  const struct I2cSlaveConfig * const config = configBase;
  const struct I2cBaseConfig baseConfig = {
      .channel = config->channel,
      .scl = config->scl,
      .sda = config->sda
  };
  struct I2cSlave * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = I2cBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->cache = malloc(config->size);
  if (!interface->cache)
    return E_MEMORY;

  interface->base.handler = interruptHandler;

  interface->callback = 0;
  interface->external = 0;
  interface->internal = 0;
  interface->size = config->size;
  interface->state = STATE_IDLE;

  LPC_I2C_Type * const reg = interface->base.reg;

  /* Clear all flags */
  reg->CONCLR = CONCLR_AAC | CONCLR_SIC | CONCLR_STAC | CONCLR_I2ENC;

#ifdef CONFIG_I2C_PM
  if ((res = pmRegister(interface, powerStateHandler)) != E_OK)
    return res;
#endif

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  /* Enable I2C interface */
  reg->CONSET = CONSET_AA | CONSET_I2EN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void i2cDeinit(void *object)
{
  struct I2cSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  reg->CONCLR = CONCLR_I2ENC; /* Disable I2C interface */

#ifdef CONFIG_SERIAL_PM
  pmUnregister(interface);
#endif

  irqDisable(interface->base.irq);
  free(interface->cache);
  I2cBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result i2cCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2cSlave * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result i2cGet(void *object, enum ifOption option, void *data)
{
  struct I2cSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  switch (option)
  {
    case IF_ADDRESS:
       *(uint32_t *)data = ADR_ADDRESS_VALUE(reg->ADR0);
      return E_OK;

    case IF_POSITION:
      *(uint32_t *)data = interface->internal;
      return E_OK;

    case IF_STATUS:
      return interface->state != STATE_IDLE ? E_BUSY : E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result i2cSet(void *object, enum ifOption option, const void *data)
{
  struct I2cSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  switch (option)
  {
    case IF_ADDRESS:
      /* TODO Check range */
      reg->ADR0 = ADR_ADDRESS(*(const uint32_t *)data);
      return E_OK;

    case IF_POSITION:
      if (*(const uint32_t *)data < interface->size)
      {
        interface->internal = *(const uint32_t *)data;
        return E_OK;
      }
      else
      {
        return E_VALUE;
      }

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static size_t i2cRead(void *object, void *buffer, size_t length)
{
  struct I2cSlave * const interface = object;
  const uint8_t * const position = interface->cache + interface->internal;
  uint16_t left = interface->size - interface->internal;

  if (!length)
    return 0;

  if (length < left)
  {
    left = length;
    interface->internal += length;
  }
  else
    interface->internal = 0;

  memcpy(buffer, position, left);

  return left;
}
/*----------------------------------------------------------------------------*/
static size_t i2cWrite(void *object, const void *buffer, size_t length)
{
  struct I2cSlave * const interface = object;
  uint8_t * const position = interface->cache + interface->internal;
  uint16_t left = interface->size - interface->internal;

  if (!length)
    return 0;

  if (length < left)
  {
    left = length;
    interface->internal += length;
  }
  else
    interface->internal = 0;

  memcpy(position, buffer, left);

  return left;
}
