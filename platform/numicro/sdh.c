/*
 * sdh.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/sdio.h>
#include <halm/generic/sdio_defs.h>
#include <halm/platform/numicro/sdh.h>
#include <halm/platform/numicro/sdh_defs.h>
#include <halm/platform/platform_defs.h>
#include <xcore/memory.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define CMD_STOP_TRANSMISSION 12
#define DEFAULT_BLOCK_SIZE    512
#define BUSY_READ_DELAY       100 /* Milliseconds */
#define BUSY_WRITE_DELAY      500 /* Milliseconds */
/*----------------------------------------------------------------------------*/
static void execute(struct Sdh *, uint32_t);
static void executeCommand(struct Sdh *, uint32_t, uint32_t);
// static void pinInterruptHandler(void *);
// static void sdioInterruptHandler(void *);
static void interruptHandler(void *);
static enum Result updateRate(struct Sdh *, uint32_t);

// TODO Power Management
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static void sdioSetCallback(void *, void (*)(void *), void *);
static enum Result sdioGetParam(void *, int, void *);
static enum Result sdioSetParam(void *, int, const void *);
static size_t sdioRead(void *, void *, size_t);
static size_t sdioWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Sdh = &(const struct InterfaceClass){
    .size = sizeof(struct Sdh),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .setCallback = sdioSetCallback,
    .getParam = sdioGetParam,
    .setParam = sdioSetParam,
    .read = sdioRead,
    .write = sdioWrite
};
/*----------------------------------------------------------------------------*/
static void execute(struct Sdh *interface, uint32_t blocks)
{
  NM_SDH_Type * const reg = interface->base.reg;
  const uint32_t code = COMMAND_CODE_VALUE(interface->command);
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  const enum SDIOResponse response = COMMAND_RESP_VALUE(interface->command);

  /* Prepare interrupt mask */
  const uint32_t waitStatus = (flags & SDIO_DATA_MODE) ?
      INTEN_BLKDIEN | INTEN_DITOIEN : 0;

  /* Initialize interrupts */
  reg->INTSTS = INTEN_BLKDIEN | INTEN_CRCIEN | INTEN_RTOIEN | INTEN_DITOIEN;
  reg->INTEN = waitStatus;

  /* Prepare command */
  uint32_t command = 0;

  if (flags & SDIO_INITIALIZE)
    command |= CTL_CLK74OEN;
  else
    command |= CTL_COEN | CTL_CLKKEEP | CTL_CMDCODE(code);

  switch (response)
  {
    case SDIO_RESPONSE_LONG:
      command |= CTL_R2EN;
      break;

    case SDIO_RESPONSE_SHORT:
      command |= CTL_RIEN;
      break;

    default:
      break;
  }

  if (flags & SDIO_DATA_MODE)
  {
    command |= CTL_BLKCNT(blocks);

    if (interface->base.wide)
      command |= CTL_DBW;

    if (flags & SDIO_WRITE_MODE)
      command |= CTL_DOEN;
    else
      command |= CTL_DIEN;
  }

  interface->status = E_BUSY;

  /* Enable interrupts when at least one interrupt source is enabled */
  if (waitStatus)
  {
    irqClearPending(interface->base.irq);
    irqEnable(interface->base.irq);
  }

  /* Execute low-level command */
  executeCommand(interface, command, interface->argument);

  /* Disable interrupts when data phase is not used */
  if (interface->status != E_BUSY)
    irqDisable(interface->base.irq);
}
/*----------------------------------------------------------------------------*/
static void executeCommand(struct Sdh *interface, uint32_t command,
    uint32_t argument)
{
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  NM_SDH_Type * const reg = interface->base.reg;

  reg->CMDARG = argument;
  reg->CTL = command;

  if (!(command & (CTL_DIEN | CTL_DOEN)))
  {
    if (command & (CTL_RIEN | CTL_R2EN))
    {
      do
      {
        const uint32_t status = reg->INTSTS;

        if (status & INTSTS_RTOIF)
        {
          /* Timeout error */
          interface->status = E_TIMEOUT;
          break;
        }

        if ((status & INTSTS_CRCIF) && (flags & SDIO_CHECK_CRC))
        {
          /* Integrity error */
          interface->status = E_INTERFACE;
          break;
        }
      }
      while (reg->CTL & (CTL_RIEN | CTL_R2EN));

      if (interface->status == E_BUSY)
        interface->status = E_OK;
    }
    else if (command & CTL_COEN)
    {
      while (reg->CTL & CTL_COEN);
      interface->status = E_OK;
    }
    else if (command & CTL_CLK74OEN)
    {
      while (reg->CTL & CTL_CLK74OEN);
      interface->status = E_OK;
    }
    else
    {
      /* Unknown command */
      assert(!(command & (CTL_COEN | CTL_RIEN | CTL_R2EN | CTL_CLK74OEN)));
    }
  }

  if (interface->status != E_OK && interface->status != E_BUSY)
  {
    reg->CTL = CTL_CTLRST;
    while (reg->CTL & CTL_CTLRST);
    reg->CTL = CTL_CLKKEEP;
  }
}
/*----------------------------------------------------------------------------*/
// static void pinInterruptHandler(void *object)
// {
//   struct Sdh * const interface = object;

//   /* Disable further DATA0 interrupts */
//   interruptDisable(interface->finalizer);

//   interface->status = E_OK;

//   if (interface->callback != NULL)
//     interface->callback(interface->callbackArgument);
// }
/*----------------------------------------------------------------------------*/
// static void sdioInterruptHandler(void *object)
static void interruptHandler(void *object)
{
  struct Sdh * const interface = object;
  NM_SDH_Type * const reg = interface->base.reg;
  const uint32_t ahbStatus = reg->GINTSTS;
  const uint32_t dmaStatus = reg->DMAINTSTS;
  const uint32_t intStatus = reg->INTSTS;

  // /* Disable further DATA0 interrupts */
  // interruptDisable(interface->finalizer);

  /* Clear pending interrupts */
  reg->GINTSTS = ahbStatus;
  reg->DMAINTSTS = dmaStatus;
  reg->INTSTS = intStatus;

  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);

  if (ahbStatus & GINTSTS_DTAIF)
  {
    /* Reset engine */
    reg->GCTL |= GCTL_GCTLRST;
    /* Host error */
    interface->status = E_ERROR;
  }
  else if ((intStatus & INTSTS_CRCIF) && (flags & SDIO_CHECK_CRC))
  {
    /* Integrity error */
    interface->status = E_INTERFACE;
  }
  else if (intStatus & (INTSTS_RTOIF | INTSTS_DITOIF))
  {
    /* Timeout error */
    interface->status = E_TIMEOUT;
  }
  else
  {
    bool completed;

    if (flags & SDIO_DATA_MODE)
    {
      completed = (intStatus & INTSTS_BLKDIF) != 0;
    }

    if (flags & SDIO_AUTO_STOP)
    {
      const uint32_t command = CTL_COEN | CTL_CLKKEEP
          | CTL_CMDCODE(CMD_STOP_TRANSMISSION);

      /* Inject Stop Transmission command */
      executeCommand(interface, command, 0);
    }

    if (completed)
    {
      if (!(reg->INTSTS & INTSTS_DAT0STS))
      {
        interface->status = E_INVALID;
  //       interruptEnable(interface->finalizer);

  //       if (pinRead(interface->data0))
  //       {
  //         interruptDisable(interface->finalizer);
  //         interface->status = E_OK;
  //       }
  //       else
  //       {
  //         /* Disable SDMMC interrupts, wait for high level on DATA0 */
  //         irqDisable(interface->base.irq);
  //       }
      }
      else
      {
        interface->status = E_OK;
      }
    }
  }

  if (interface->status != E_BUSY)
  {
    /* Disable SDMMC interrupts */
    irqDisable(interface->base.irq);

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result updateRate(struct Sdh *interface, uint32_t rate)
{
  NM_SDH_Type * const reg = interface->base.reg;
  const uint32_t clock = sdhGetClock(&interface->base);

  if (rate <= clock)
  {
    /* Update clock divider */
    const uint32_t divider = (clock + rate - 1) / rate;
    sdhSetDivider(&interface->base, divider);

    /* Update timeout values */
    const uint32_t timeout = BUSY_READ_DELAY * (rate / 1000);
    reg->TOUT = MIN(timeout, TOUT_TOUT_MAX);

    interface->rate = rate;
    return E_OK;
  }
  else
    return E_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdhConfig * const config = configBase;
  assert(config);
  assert(config->rate);

  // const struct PinIntConfig finalizerConfig = {
  //     .pin = config->dat0,
  //     .event = PIN_RISING,
  //     .pull = PIN_NOPULL,
  //     .priority = config->priority
  // };
  const struct SdhBaseConfig baseConfig = {
      .clk = config->clk,
      .cmd = config->cmd,
      .dat0 = config->dat0,
      .dat1 = config->dat1,
      .dat2 = config->dat2,
      .dat3 = config->dat3,
      .channel = config->channel
  };
  struct Sdh * const interface = object;
  enum Result res;

  // interface->finalizer = init(PinInt, &finalizerConfig);
  // if (!interface->finalizer)
  //   return E_ERROR;
  // interruptSetCallback(interface->finalizer, pinInterruptHandler, interface);

  /* Call base class constructor */
  if ((res = SdhBase->init(interface, &baseConfig)) != E_OK)
    return res;

  // interface->base.handler = sdioInterruptHandler;
  interface->base.handler = interruptHandler;
  interface->argument = 0;
  interface->callback = NULL;
  interface->command = 0;
  interface->status = E_OK;

  NM_SDH_Type * const reg = interface->base.reg;

  reg->DMACTL = DMACTL_DMARST;
  while (reg->DMACTL & DMACTL_DMARST);
  reg->DMACTL = DMACTL_DMAEN;

  reg->GCTL = GCTL_GCTLRST | GCTL_SDEN;
  while (reg->GCTL & GCTL_GCTLRST);

  reg->CTL = CTL_CTLRST;
  while (reg->CTL & CTL_CTLRST);
  reg->CTL = CTL_CLKKEEP;

  /* Enable global interrupts */
  reg->GINTEN = GINTEN_DTAIEN;
  /* Disable control interrupts */
  reg->INTEN = 0;
  /* Set default block size */
  reg->BLEN = DEFAULT_BLOCK_SIZE - 1;

  /* Set default clock rate */
  updateRate(interface, config->rate);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  struct Sdh * const interface = object;
  NM_SDH_Type * const reg = interface->base.reg;

  reg->INTEN = 0;
  reg->GINTEN = 0;
  reg->CTL = 0;
  reg->GCTL = 0;
  reg->DMACTL = 0;

  // deinit(interface->finalizer);

  SdhBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static void sdioSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Sdh * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioGetParam(void *object, int parameter, void *data)
{
  struct Sdh * const interface = object;
  NM_SDH_Type * const reg = interface->base.reg;

  /* Additional options */
  switch ((enum SDIOParameter)parameter)
  {
    case IF_SDIO_MODE:
    {
      *(uint8_t *)data = interface->base.wide ? SDIO_4BIT : SDIO_1BIT;
      return E_OK;
    }

    case IF_SDIO_RESPONSE:
    {
      const enum SDIOResponse response = COMMAND_RESP_VALUE(interface->command);
      uint32_t * const buffer = data;

      if (response == SDIO_RESPONSE_LONG)
      {
        uint32_t received[5];

        for (size_t index = 0; index < ARRAY_SIZE(received); ++index)
          received[index] = fromBigEndian32(reg->FB[index]);

        for (size_t index = 0; index < ARRAY_SIZE(received) - 1; ++index)
        {
          buffer[ARRAY_SIZE(received) - 2 - index] =
              (received[index] << 8) | (received[index + 1] >> 24);
        }

        return E_OK;
      }
      else if (response == SDIO_RESPONSE_SHORT)
      {
        /*
         * RESP0 contains bits 47..16 of the response token
         * RESP1 contains bits 15..8 of the response token
         */
        buffer[0] = (reg->RESP0 << 8) | (reg->RESP1 & 0xFF);
        return E_OK;
      }
      else
        return E_ERROR;
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
      *(uint32_t *)data = interface->rate;
      return E_OK;

    case IF_STATUS:
      return interface->status;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result sdioSetParam(void *object, int parameter, const void *data)
{
  struct Sdh * const interface = object;
  NM_SDH_Type * const reg = interface->base.reg;

  /* Additional options */
  switch ((enum SDIOParameter)parameter)
  {
    case IF_SDIO_EXECUTE:
      execute(interface, 0);
      return interface->status;

    case IF_SDIO_ARGUMENT:
      interface->argument = *(const uint32_t *)data;
      return E_OK;

    case IF_SDIO_BLOCK_SIZE:
    {
      const uint32_t blockLength = *(const uint32_t *)data;

      if (blockLength < 0xFFFF)
      {
        reg->BLEN = blockLength - 1;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    case IF_SDIO_COMMAND:
      interface->command = *(const uint32_t *)data;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
      return updateRate(interface, *(const uint32_t *)data);

    case IF_ZEROCOPY:
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t sdioRead(void *object, void *buffer, size_t length)
{
  struct Sdh * const interface = object;
  NM_SDH_Type * const reg = interface->base.reg;

  reg->DMASA = (uint32_t)buffer;

  execute(interface, length / (reg->BLEN + 1));
  return length;
}
/*----------------------------------------------------------------------------*/
static size_t sdioWrite(void *object, const void *buffer, size_t length)
{
  struct Sdh * const interface = object;
  NM_SDH_Type * const reg = interface->base.reg;

  reg->DMASA = (uint32_t)buffer;

  execute(interface, length / (reg->BLEN + 1));
  return length;
}
