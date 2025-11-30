/*
 * i2c.c
 * Copyright (C) 2025 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/i2c.h>
#include <halm/platform/lpc/gen_2/i2c_defs.h>
#include <halm/platform/lpc/i2c.h>
#include <halm/pm.h>
#include <assert.h>
#include <limits.h>
/*----------------------------------------------------------------------------*/
enum State
{
  STATE_IDLE,
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
  const uint8_t status = reg->STAT;
  bool event = false;

  if (status & STAT_MSK_ERROR_MASK)
  {
    reg->STAT = STAT_MSK_ERROR_MASK;
    interface->state = STATE_ERROR;
    event = true;
  }
  else if (status & STAT_MSTPENDING)
  {
    const uint8_t state = STAT_MSTSTATE_VALUE(status);

    switch (state)
    {
      case MSTSTATE_RX_READY:
        *(uint8_t *)interface->buffer = reg->MSTDAT;
        ++interface->buffer;
        --interface->left;

        if (!interface->left)
        {
          reg->MSTCTL = MSTCTL_MSTSTOP;
          interface->state = STATE_IDLE;
          event = true;
        }
        else
          reg->MSTCTL = MSTCTL_MSTCONTINUE;
        break;

      case MSTSTATE_TX_READY:
        if (interface->left > 0)
        {
          reg->MSTDAT = *(const uint8_t *)interface->buffer;
          reg->MSTCTL = MSTCTL_MSTCONTINUE;

          ++interface->buffer;
          --interface->left;
        }
        else
        {
          if (!interface->sendRepeatedStart)
            reg->MSTCTL = MSTCTL_MSTSTOP;

          interface->sendRepeatedStart = false;
          interface->state = STATE_IDLE;
          event = true;
        }
        break;

      case MSTSTATE_NACK_DATA:
      case MSTSTATE_NACK_ADDRESS:
        reg->MSTCTL = MSTCTL_MSTSTOP;

        interface->sendRepeatedStart = false;
        interface->state = STATE_ERROR;
        event = true;
        break;

      default:
        break;
    }
  }

  if (interface->state == STATE_IDLE || interface->state == STATE_ERROR)
  {
    reg->INTENCLR = INTENCLR_MSTPENDINGCLR;
    irqDisable(interface->base.irq);
  }

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

  /* Enable interrupts */
  reg->INTENSET = INTENSET_MSTARBLOSSEN | INTENSET_MSTSTSTPERREN
      | INTENSET_EVENTTIMEOUTEN | INTENSET_SCLTIMEOUTEN;
  /* Configure timeout */
  reg->TIMEOUT = TIMEOUT_TOMIN(0xF) | TIMEOUT_TO(625);

#ifdef CONFIG_PLATFORM_LPC_I2C_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Enable the interface */
  reg->CFG = CFG_MSTEN | CFG_TIMEOUTEN;

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
  reg->CFG = 0;

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

      reg->MSTCTL = MSTCTL_MSTSTOP;
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
  interface->left = length;
  interface->state = STATE_RECEIVE;

  /* Clear pending interrupt flags, enable interrupt */
  reg->STAT = STAT_MST_MASK;
  irqEnable(interface->base.irq);

  /* Send address byte */
  reg->MSTDAT = (interface->address << 1) | MSTDAT_READ;
  reg->MSTCTL = MSTCTL_MSTSTART;
  reg->INTENSET = INTENSET_MSTPENDINGEN;

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
  interface->left = length;
  interface->state = STATE_TRANSMIT;

  /* Stop previous transmission when Repeated Start is enabled */
  if (STAT_MSTSTATE_VALUE(reg->STAT) == MSTSTATE_TX_READY)
    reg->MSTCTL = MSTCTL_MSTSTOP;

  /* Clear pending interrupt flags, enable interrupt */
  reg->STAT = STAT_MST_MASK;
  irqEnable(interface->base.irq);

  /* Send address byte */
  reg->MSTDAT = (interface->address << 1) | MSTDAT_WRITE;
  reg->MSTCTL = MSTCTL_MSTSTART;
  reg->INTENSET = INTENSET_MSTPENDINGEN;

  if (interface->blocking)
  {
    while (interface->state != STATE_IDLE && interface->state != STATE_ERROR)
      barrier();

    if (interface->state == STATE_ERROR)
      return 0;
  }

  return length;
}
