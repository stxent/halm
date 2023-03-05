/*
 * i2c.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/numicro/i2c.h>
#include <halm/platform/numicro/i2c_defs.h>
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
  STATUS_BUS_ERROR              = 0x00,
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
  STATUS_DATA_RECEIVED_NACK     = 0x58,
  STATUS_NO_INFORMATION         = 0xF8
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
  const uint8_t stat = reg->STATUS0;
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
      reg->CTL0 &= ~CTL0_STA;
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
        reg->DAT = *interface->txBuffer++;
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
          reg->CTL0 |= CTL0_STO;

        interface->state = STATE_IDLE;
        event = true;
      }
      break;

    /* Data has been received and ACK has been returned */
    case STATUS_DATA_RECEIVED_ACK:
      --interface->rxLeft;
      *interface->rxBuffer++ = reg->DAT;

      if (interface->rxLeft > 1)
        reg->CTL0 |= CTL0_AA;
      else
        reg->CTL0 &= ~CTL0_AA;
      break;

    /* Last byte of data has been received and NACK has been returned */
    case STATUS_DATA_RECEIVED_NACK:
      if (interface->state != STATE_RECEIVE)
        break;

      --interface->rxLeft;
      *interface->rxBuffer++ = reg->DAT;

      reg->CTL0 |= CTL0_STO;
      interface->state = STATE_IDLE;
      event = true;
      break;

    /* External interference disturbed the internal signals */
    case STATUS_BUS_ERROR:
    /* Arbitration has been lost during transmission or reception */
    case STATUS_ARBITRATION_LOST:
    /* Data byte has been transmitted, NACK received */
    case STATUS_DATA_TRANSMITTED_NACK:
    /* Address and direction bit have not been acknowledged */
    case STATUS_SLAVE_WRITE_NACK:
    case STATUS_SLAVE_READ_NACK:
      interface->sendRepeatedStart = false;

      reg->CTL0 |= CTL0_STO;
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
    reg->CTL0 |= CTL0_SI;

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
  interface->extended = config->extended;
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
      i2cSetRate(&interface->base, *(const uint32_t *)data);
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

  interface->rxLeft = length;
  interface->txLeft = 0;
  interface->rxBuffer = buffer;
  interface->txBuffer = 0;

  interface->state = STATE_ADDRESS;
  reg->CTL0 |= CTL0_STA;

  /* Continue previous transmission when Repeated Start is enabled */
  if (reg->STATUS0 == STATUS_DATA_TRANSMITTED_ACK)
    reg->CTL0 |= CTL0_SI;

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

  interface->rxLeft = 0;
  interface->txLeft = length;
  interface->rxBuffer = 0;
  interface->txBuffer = buffer;

  /* Stop previous transmission when Repeated Start is enabled */
  if (reg->STATUS0 == STATUS_DATA_TRANSMITTED_ACK)
    reg->CTL0 |= CTL0_SI | CTL0_STO;

  /*
   * Start condition generation can be delayed when the bus is not free.
   * After Stop condition being transmitted Start will be generated
   * automatically.
   */
  interface->state = STATE_ADDRESS;
  reg->CTL0 |= CTL0_STA;

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
