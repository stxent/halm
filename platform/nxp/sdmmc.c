/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/generic/sdio.h>
#include <halm/generic/sdio_defs.h>
#include <halm/platform/nxp/dma_sdmmc.h>
#include <halm/platform/nxp/sdmmc.h>
#include <halm/platform/nxp/sdmmc_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_BLOCK_SIZE  512
#define BUSY_READ_DELAY     100 /* Milliseconds */
#define BUSY_WRITE_DELAY    500 /* Milliseconds */
/*----------------------------------------------------------------------------*/
static void execute(struct Sdmmc *);
static enum Result executeCommand(struct Sdmmc *, uint32_t, uint32_t);
static void interruptHandler(void *);
static enum Result updateRate(struct Sdmmc *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static enum Result sdioSetCallback(void *, void (*)(void *), void *);
static enum Result sdioGetParam(void *, enum IfParameter, void *);
static enum Result sdioSetParam(void *, enum IfParameter, const void *);
static size_t sdioRead(void *, void *, size_t);
static size_t sdioWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Sdmmc = &(const struct InterfaceClass){
    .size = sizeof(struct Sdmmc),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .setCallback = sdioSetCallback,
    .getParam = sdioGetParam,
    .setParam = sdioSetParam,
    .read = sdioRead,
    .write = sdioWrite
};
/*----------------------------------------------------------------------------*/
static void execute(struct Sdmmc *interface)
{
  LPC_SDMMC_Type * const reg = interface->base.reg;
  const uint32_t code = COMMAND_CODE_VALUE(interface->command);
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  const enum SdioResponse response = COMMAND_RESP_VALUE(interface->command);

  /* Prepare interrupt mask */
  uint32_t waitStatus = INT_EBE | INT_SBE | INT_HLE | INT_RTO | INT_RE;

  if (flags & SDIO_INITIALIZE)
    waitStatus |= INT_CDONE;
  if (flags & SDIO_DATA_MODE)
  {
    /* Enable data-relative interrupts */
    waitStatus |= INT_DTO | INT_DRTO | INT_HTO | INT_FRUN;
    if (flags & SDIO_CHECK_CRC)
      waitStatus |= INT_DCRC;
  }
  if (!(flags & SDIO_DATA_MODE) && response != SDIO_RESPONSE_NONE)
    waitStatus |= INT_CDONE;
  if (flags & SDIO_CHECK_CRC)
    waitStatus |= INT_RCRC;

  /* Initialize interrupts */
  reg->RINTSTS = INT_MASK;
  irqClearPending(interface->base.irq);
  reg->INTMASK = waitStatus;
  irqEnable(interface->base.irq);

  /* Prepare command */
  uint32_t command = code;

  if (flags & SDIO_INITIALIZE)
    command |= CMD_SEND_INITIALIZATION;
  if (flags & SDIO_CHECK_CRC)
    command |= CMD_CHECK_RESPONSE_CRC;
  if (flags & SDIO_DATA_MODE)
    command |= CMD_DATA_EXPECTED;
  switch (response)
  {
    case SDIO_RESPONSE_LONG:
      command |= CMD_RESPONSE_LENGTH;
      /* Falls through */
    case SDIO_RESPONSE_SHORT:
      command |= CMD_RESPONSE_EXPECT;
      break;

    default:
      break;
  }
  if (flags & SDIO_WRITE_MODE)
    command |= CMD_READ_WRITE;
  if (flags & SDIO_STOP_TRANSFER)
    command |= CMD_STOP_ABORT_CMD;
  if (flags & SDIO_AUTO_STOP)
    command |= CMD_SEND_AUTO_STOP;
  if (flags & (SDIO_WAIT_DATA | SDIO_DATA_MODE))
    command |= CMD_WAIT_PRVDATA_COMPLETE;

  /* Reset FIFO when data mode is requested */
  if (flags & SDIO_DATA_MODE)
  {
    reg->CTRL |= CTRL_FIFO_RESET;
    while (reg->CTRL & CTRL_FIFO_RESET);
  }

  /* Execute low-level command */
  interface->status = E_BUSY;
  if (executeCommand(interface, command, interface->argument) != E_OK)
    interface->status = E_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum Result executeCommand(struct Sdmmc *interface, uint32_t command,
    uint32_t argument)
{
  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* TODO Add timeout */
  reg->CMDARG = argument;
  reg->CMD = command | CMD_START;

  /* Poll until command is accepted by the CIU */
  while (reg->CMD & CMD_START);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  static const uint32_t hostErrors = INT_FRUN | INT_HLE;
  static const uint32_t integrityErrors = INT_RE | INT_RCRC | INT_DCRC
      | INT_SBE | INT_EBE;
  static const uint32_t timeoutErrors = INT_RTO | INT_DRTO | INT_HTO;

  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;
  const uint32_t status = reg->MINTSTS;

  interruptDisable(interface->finalizer);

  irqDisable(interface->base.irq);
  reg->INTMASK = 0;
  reg->RINTSTS = INT_MASK;

  if (status & hostErrors)
  {
    interface->status = E_ERROR;
  }
  else if (status & integrityErrors)
  {
    interface->status = E_INTERFACE;
  }
  else if (status & timeoutErrors)
  {
    interface->status = E_TIMEOUT;
  }
  else if (!(reg->STATUS & STATUS_DATA_BUSY))
  {
    interface->status = E_OK;
  }
  else
  {
    interruptEnable(interface->finalizer);

    if (!(reg->STATUS & STATUS_DATA_BUSY))
    {
      interface->status = E_OK;
      interruptDisable(interface->finalizer);
    }
    else
      interface->status = E_BUSY;
  }

  if (interface->status != E_BUSY && interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result updateRate(struct Sdmmc *interface, uint32_t rate)
{
  LPC_SDMMC_Type * const reg = interface->base.reg;
  const uint32_t clock = sdmmcGetClock(&interface->base);

  if (rate > clock)
    return E_VALUE;

  const uint32_t current = CLKDIV_VALUE(0, reg->CLKDIV);
  const uint32_t divider = ((clock + (rate >> 1)) / rate) >> 1;

  if (divider == current && (reg->CLKENA & CLKENA_CCLK_ENABLE))
    return E_OK; /* Closest rate is already set */

  /* Disable clock and reset divider */
  reg->CLKENA = 0;
  reg->CLKSRC = 0;
  executeCommand(interface, CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  /* Set divider 0 to desired value */
  reg->CLKDIV = (reg->CLKDIV & ~CLKDIV_MASK(0))
      | CLKDIV_DIVIDER(0, divider);
  executeCommand(interface, CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  /* Update timeout values */
  const uint32_t readTimeout = BUSY_READ_DELAY * (rate / 1000);
  reg->TMOUT = TMOUT_RESPONSE_TIMEOUT(0x40) | TMOUT_DATA_TIMEOUT(readTimeout);

  /* Enable clock */
  reg->CLKENA = CLKENA_CCLK_ENABLE;
  executeCommand(interface, CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  interface->rate = rate;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcConfig * const config = configBase;
  assert(config);

  const struct DmaSdmmcConfig dmaConfig = {
      .burst = DMA_BURST_4,
      .number = 16,
      .parent = object
  };
  const struct PinInterruptConfig finalizerConfig = {
      .pin = config->dat0,
      .event = PIN_RISING,
      .pull = PIN_NOPULL,
      .priority = config->priority
  };
  const struct SdmmcBaseConfig baseConfig = {
      .clk = config->clk,
      .cmd = config->cmd,
      .dat0 = config->dat0,
      .dat1 = config->dat1,
      .dat2 = config->dat2,
      .dat3 = config->dat3
  };
  struct Sdmmc * const interface = object;
  enum Result res;

  assert(config->rate);

  interface->finalizer = init(PinInterrupt, &finalizerConfig);
  if (!interface->finalizer)
    return E_ERROR;
  interruptSetCallback(interface->finalizer, interruptHandler, interface);

  /* Call base class constructor */
  if ((res = SdmmcBase->init(object, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = interruptHandler;
  interface->argument = 0;
  interface->callback = 0;
  interface->command = 0;
  interface->status = E_OK;

  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* Reset specified blocks */
  reg->CTRL = CTRL_CONTROLLER_RESET;
  while (reg->CTRL & CTRL_CONTROLLER_RESET);

  /* Enable interrupts */
  reg->INTMASK = 0;
  reg->CTRL = CTRL_INT_ENABLE;
  irqSetPriority(interface->base.irq, config->priority);
  /* Clear pending interrupts */
  reg->RINTSTS = INT_MASK;

  /* Set bus width */
  reg->CTYPE = interface->base.wide ? CTYPE_CARD_WIDTH0_4BIT : 0;
  /* Set default block size */
  reg->BLKSIZ = DEFAULT_BLOCK_SIZE;

  /* Set default clock rate */
  updateRate(interface, config->rate);

  /* Internal DMA controller should be initialized after interface setup */
  interface->dma = init(DmaSdmmc, &dmaConfig);
  if (!interface->dma)
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  deinit(interface->dma);

  /* Disable interrupts */
  reg->CTRL &= ~CTRL_INT_ENABLE;

  deinit(interface->finalizer);

  SdmmcBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum Result sdioSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Sdmmc * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* Additional options */
  switch ((enum SdioParameter)parameter)
  {
    case IF_SDIO_MODE:
    {
      *(uint8_t *)data = interface->base.wide ? SDIO_4BIT : SDIO_1BIT;
      return E_OK;
    }

    case IF_SDIO_RESPONSE:
    {
      const enum SdioResponse response = COMMAND_RESP_VALUE(interface->command);
      uint32_t * const buffer = data;

      if (response == SDIO_RESPONSE_LONG)
      {
        for (size_t index = 0; index < 4; ++index)
          buffer[index] = reg->RESP[index];

        return E_OK;
      }
      else if (response == SDIO_RESPONSE_SHORT)
      {
        buffer[0] = reg->RESP[0];
        return E_OK;
      }
      else
        return E_ERROR;
    }

    default:
      break;
  }

  switch (parameter)
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
static enum Result sdioSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* Additional options */
  switch ((enum SdioParameter)parameter)
  {
    case IF_SDIO_EXECUTE:
      execute(interface);
      return E_BUSY;

    case IF_SDIO_ARGUMENT:
      interface->argument = *(const uint32_t *)data;
      return E_OK;

    case IF_SDIO_BLOCK_SIZE:
    {
      const uint32_t blockLength = *(const uint32_t *)data;

      if (blockLength < 0xFFFF)
      {
        reg->BLKSIZ = blockLength;
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

  switch (parameter)
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
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  reg->BYTCNT = length;
  dmaAppend(interface->dma, buffer, 0, length);

  const enum Result res = dmaEnable(interface->dma);

  if (res == E_OK)
  {
    execute(interface);
    return length;
  }
  else
  {
    interface->status = res;
    return 0;
  }
}
/*----------------------------------------------------------------------------*/
static size_t sdioWrite(void *object, const void *buffer, size_t length)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  reg->BYTCNT = length;
  dmaAppend(interface->dma, 0, buffer, length);

  const enum Result res = dmaEnable(interface->dma);

  if (res == E_OK)
  {
    execute(interface);
    return length;
  }
  else
  {
    interface->status = res;
    return 0;
  }
}
