/*
 * i2c.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/numicro/i2c.h>
#include <halm/platform/numicro/i2c_defs.h>
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

#ifdef CONFIG_PLATFORM_NUMICRO_I2C_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result i2cInit(void *, const void *);
static void i2cSetCallback(void *, void (*)(void *), void *);
static enum Result i2cGetParam(void *, int, void *);
static enum Result i2cSetParam(void *, int, const void *);
static size_t i2cRead(void *, void *, size_t);
static size_t i2cWrite(void *, const void *, size_t);

#ifndef CONFIG_PLATFORM_NUMICRO_I2C_NO_DEINIT
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
  NM_I2C_Type * const reg = interface->base.reg;
  uint8_t status = reg->STATUS0;
  bool clear = true;
  bool event = false;

  if (reg->TOCTL & TOCTL_TOIF)
  {
    reg->TOCTL |= TOCTL_TOIF;

    interface->state = STATE_ERROR;
    event = true;
    status = STATUS_BUS_RELEASED;
  }

  switch (status)
  {
    /* Start or Repeated Start conditions have been transmitted */
    case STATUS_START_TRANSMITTED:
    case STATUS_RESTART_TRANSMITTED:
      if (interface->state == STATE_ADDRESS)
      {
        reg->DAT = (interface->txLeft ? DATA_WRITE : DATA_READ)
            | (interface->address << 1);
      }
      reg->CTL0 &= ~CTL0_STA;
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
        reg->CTL0 |= CTL0_AA; /* Assert ACK after data is received */
      else
        reg->CTL0 &= ~CTL0_AA; /* Assert NACK after data is received */
      interface->state = STATE_RECEIVE;
      break;

    /* Data byte has been transmitted, ACK received */
    case STATUS_DATA_TRANSMITTED_ACK:
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
          clear = false;
        }
        else
          reg->CTL0 |= CTL0_STO;

        interface->state = STATE_IDLE;
        event = true;
      }
      break;

    /* Data has been received and ACK has been returned */
    case STATUS_DATA_RECEIVED_ACK:
      *(uint8_t *)interface->buffer++ = reg->DAT;
      ++interface->buffer;
      --interface->rxLeft;

      if (interface->rxLeft > 1)
        reg->CTL0 |= CTL0_AA;
      else
        reg->CTL0 &= ~CTL0_AA;
      break;

    /* Last byte of data has been received and NACK has been returned */
    case STATUS_DATA_RECEIVED_NACK:
      if (interface->state != STATE_RECEIVE)
        break;

      *(uint8_t *)interface->buffer++ = reg->DAT;
      ++interface->buffer;
      --interface->rxLeft;

      reg->CTL0 |= CTL0_STO;
      interface->state = STATE_IDLE;
      event = true;
      break;

    case STATUS_BUS_ERROR:
    case STATUS_ARBITRATION_LOST:
    case STATUS_DATA_TRANSMITTED_NACK:
    case STATUS_SLAVE_WRITE_NACK:
    case STATUS_SLAVE_READ_NACK:
      reg->CTL0 |= CTL0_STO;

      interface->sendRepeatedStart = false;
      interface->state = STATE_ERROR;
      event = true;
      break;

    case STATUS_BUS_RELEASED:
      clear = false;
      break;

    default:
      break;
  }

  if (clear)
    reg->CTL0 |= CTL0_SI;

  if (interface->state == STATE_IDLE || interface->state == STATE_ERROR)
  {
    irqDisable(interface->base.irq);
    reg->TOCTL = 0;
  }

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_I2C_PM
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
  interface->extended = config->extended;
  interface->rate = config->rate;
  interface->sendRepeatedStart = false;
  interface->state = STATE_IDLE;

  NM_I2C_Type * const reg = interface->base.reg;

  /* Disable the interface */
  reg->CTL0 = 0;

  /* Configure timings */
  i2cSetRate(&interface->base, config->rate);

  reg->CTL1 = interface->extended ? CTL1_ADDR10EN : 0;
  reg->TOCTL = 0;
  reg->WKCTL = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_I2C_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the interface */
  reg->CTL0 = CTL0_I2CEN | CTL0_INTEN;

  irqSetPriority(interface->base.irq, config->priority);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_I2C_NO_DEINIT
static void i2cDeinit(void *object)
{
  struct I2C * const interface = object;
  NM_I2C_Type * const reg = interface->base.reg;

  /* Disable the interface */
  reg->CTL0 = 0;

#ifdef CONFIG_PLATFORM_NUMICRO_I2C_PM
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
    case IF_STATUS:
      if (interface->state != STATE_ERROR)
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
    case IF_I2C_REPEATED_START:
      interface->sendRepeatedStart = true;
      return E_OK;

    case IF_I2C_BUS_RECOVERY:
    {
      NM_I2C_Type * const reg = interface->base.reg;

      reg->CTL0 |= CTL0_STO;
      i2cRecoverBus(&interface->base);
      i2cConfigPins(&interface->base);
      interface->sendRepeatedStart = false;

      return E_OK;
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_ADDRESS:
    {
      const uint32_t address = *(const uint32_t *)data;

      if ((interface->extended && address <= 1023) || address <= 127)
      {
        interface->address = address;
        return E_OK;
      }
      else
        return E_VALUE;
    }

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
  NM_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;
  if (length > USHRT_MAX)
    length = USHRT_MAX;

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = length;
  interface->txLeft = 0;
  interface->state = STATE_ADDRESS;

  /* Continue previous transmission when Repeated Start is enabled */
  if (reg->STATUS0 == STATUS_DATA_TRANSMITTED_ACK)
    reg->CTL0 |= CTL0_SI | CTL0_STA;
  else
    reg->CTL0 |= CTL0_STA;

  reg->TOCTL = TOCTL_TOIF | TOCTL_TOCEN;
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
  NM_I2C_Type * const reg = interface->base.reg;

  if (!length)
    return 0;
  if (length > USHRT_MAX)
    length = USHRT_MAX;

  interface->buffer = (uintptr_t)buffer;
  interface->rxLeft = 0;
  interface->txLeft = length;
  interface->state = STATE_ADDRESS;

  /*
   * Start condition generation can be delayed when the bus is not free.
   * After Stop condition being transmitted Start will be generated
   * automatically.
   */
  reg->CTL0 |= CTL0_STA;

  reg->TOCTL = TOCTL_TOIF | TOCTL_TOCEN;
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
