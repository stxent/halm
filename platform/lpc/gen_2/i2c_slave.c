/*
 * i2c_slave.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gen_2/i2c_defs.h>
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
#  define i2cDeinit deletedDestructorTrap
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
  const uint32_t status = reg->STAT;
  bool event = false;

  if (status & STAT_SLVDESEL)
  {
    reg->STAT = STAT_SLVDESEL;
    reg->SLVCTL = 0;

    if (interface->state == STATE_DATA)
      event = true;
    interface->state = STATE_IDLE;
  }
  else if (status & (STAT_EVENTTIMEOUT | STAT_SCLTIMEOUT))
  {
    reg->STAT = STAT_EVENTTIMEOUT | STAT_SCLTIMEOUT;
    reg->SLVCTL = 0;

    interface->state = STATE_IDLE;
  }
  else if (status & STAT_SLVPENDING)
  {
    const uint8_t state = STAT_SLVSTATE_VALUE(status);

    switch (state)
    {
      case SLVSTATE_ADDRESS:
        reg->SLVCTL = SLVCTL_SLVCONTINUE;
        interface->state = STATE_ADDRESS;
        break;

      case SLVSTATE_RX_READY:
        if (interface->state == STATE_ADDRESS)
        {
          interface->external = reg->SLVDAT;
          reg->SLVCTL = SLVCTL_SLVCONTINUE;

          if (interface->external >= interface->size)
            interface->external = 0;

          interface->state = STATE_DATA;
        }
        else
        {
          interface->cache[interface->external++] = reg->SLVDAT;
          reg->SLVCTL = SLVCTL_SLVCONTINUE;

          if (interface->external >= interface->size)
            interface->external = 0;
        }
        break;

      case SLVSTATE_TX_READY:
        reg->SLVDAT = interface->cache[interface->external];
        reg->SLVCTL = SLVCTL_SLVCONTINUE;

        /* Wrap current external position after reaching the end of cache */
        if (++interface->external >= interface->size)
          interface->external = 0;
        break;

      default:
        break;
    }
  }

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *object, const void *configBase)
{
  const struct I2CSlaveConfig * const config = configBase;
  assert(config != NULL);

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
  if (interface->cache == NULL)
    return E_MEMORY;
  memset(interface->cache, 0, config->size);

  interface->base.handler = interruptHandler;

  interface->callback = NULL;
  interface->external = 0;
  interface->internal = 0;
  interface->size = config->size;
  interface->state = STATE_IDLE;

  LPC_I2C_Type * const reg = interface->base.reg;

  /* Enable interrupts */
  reg->INTENSET = INTENSET_SLVPENDINGEN | INTENSET_SLVDESELEN;
  /* Configure timeout */
  reg->TIMEOUT = TIMEOUT_TOMIN(0xF) | TIMEOUT_TO(625);
  /* Reset slave address */
  reg->SLVADR[0] = SLVADR_SADISABLE;
  reg->SLVQUAL0 = 0;

  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  /* Enable I2C interface */
  reg->CFG = CFG_SLVEN | CFG_TIMEOUTEN;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  struct I2CSlave * const interface = object;
  LPC_I2C_Type * const reg = interface->base.reg;

  /* Disable the interface */
  reg->CFG = 0;

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
       *(uint32_t *)data = SLVADR_SLVADR_VALUE(reg->SLVADR[0]);
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
        reg->SLVADR[0] = SLVADR_SLVADR(*(const uint32_t *)data);
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
  uint8_t *out = buffer;

  while (length)
  {
    size_t left = interface->size - interface->internal;
    size_t chunk = MIN(length, left);

    memcpy(out, interface->cache + interface->internal, chunk);

    interface->internal += chunk;
    length -= chunk;
    out += chunk;

    if (interface->internal == interface->size)
      interface->internal = 0;
  }

  return (size_t)(out - (const uint8_t *)buffer);
}
/*----------------------------------------------------------------------------*/
static size_t i2cWrite(void *object, const void *buffer, size_t length)
{
  struct I2CSlave * const interface = object;
  const uint8_t *in = buffer;

  while (length)
  {
    size_t left = interface->size - interface->internal;
    size_t chunk = MIN(length, left);

    memcpy(interface->cache + interface->internal, in, chunk);

    interface->internal += chunk;
    length -= chunk;
    in += chunk;

    if (interface->internal == interface->size)
      interface->internal = 0;
  }

  return (size_t)(in - (const uint8_t *)buffer);
}
