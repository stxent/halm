/*
 * i2c_slave.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_1/i2c_defs.h>
#include <halm/platform/lpc/i2c_slave.h>
#include <xcore/memory.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
  STATE_ADDRESS,
  STATE_DATA
};
/*----------------------------------------------------------------------------*/
/* Slave receiver and transmitter modes except general call modes */
enum Status
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
const struct InterfaceClass * const I2CSlave = &(const struct InterfaceClass){
    .size = sizeof(struct I2CSlave),
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
  struct I2CSlave * const interface = object;
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
      /* Falls through */
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
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CSlaveConfig * const config = configBase;
  assert(config);

  const struct I2CBaseConfig baseConfig = {
      .channel = config->channel,
      .scl = config->scl,
      .sda = config->sda
  };
  struct I2CSlave * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = I2CBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->cache = malloc(config->size);
  if (!interface->cache)
    return E_MEMORY;
  memset(interface->cache, 0, config->size);

  interface->base.handler = interruptHandler;

  interface->callback = 0;
  interface->external = 0;
  interface->internal = 0;
  interface->size = config->size;
  interface->state = STATE_IDLE;

  LPC_I2C_Type * const reg = interface->base.reg;

  /* Clear all flags */
  reg->CONCLR = CONCLR_AAC | CONCLR_SIC | CONCLR_STAC | CONCLR_I2ENC;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  /* Enable I2C interface */
  reg->CONSET = CONSET_AA | CONSET_I2EN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  struct I2CSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  reg->CONCLR = CONCLR_I2ENC; /* Disable I2C interface */

  irqDisable(interface->base.irq);
  free(interface->cache);
  I2CBase->deinit(interface);
}
#endif
/*----------------------------------------------------------------------------*/
static void i2cSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct I2CSlave * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result i2cGetParam(void *object, int parameter, void *data)
{
  struct I2CSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
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
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result i2cSetParam(void *object, int parameter, const void *data)
{
  struct I2CSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
      if (*(const uint32_t *)data <= 127)
      {
        reg->ADR0 = ADR_ADDRESS(*(const uint32_t *)data);
        return E_OK;
      }
      else
        return E_VALUE;

    case IF_POSITION:
      if (*(const uint32_t *)data < interface->size)
      {
        interface->internal = *(const uint32_t *)data;
        return E_OK;
      }
      else
        return E_VALUE;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t i2cRead(void *object, void *buffer, size_t length)
{
  struct I2CSlave * const interface = object;
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
  struct I2CSlave * const interface = object;
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
