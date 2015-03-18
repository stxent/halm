/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <common/sdio.h>
#include <common/sdio_defs.h>
#include <platform/platform_defs.h>
#include <platform/nxp/dma_sdmmc.h>
#include <platform/nxp/sdmmc.h>
#include <platform/nxp/sdmmc_defs.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_BLOCK_SIZE 512
/*----------------------------------------------------------------------------*/
static void execute(struct Sdmmc *);
static enum result executeCommand(struct Sdmmc *, uint32_t, uint32_t);
static enum result updateRate(struct Sdmmc *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static enum result sdioCallback(void *, void (*)(void *), void *);
static enum result sdioGet(void *, enum ifOption, void *);
static enum result sdioSet(void *, enum ifOption, const void *);
static uint32_t sdioRead(void *, uint8_t *, uint32_t);
static uint32_t sdioWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass sdioTable = {
    .size = sizeof(struct Sdmmc),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .callback = sdioCallback,
    .get = sdioGet,
    .set = sdioSet,
    .read = sdioRead,
    .write = sdioWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Sdmmc = &sdioTable;
/*----------------------------------------------------------------------------*/
static void execute(struct Sdmmc *interface)
{
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  const uint32_t code = COMMAND_CODE_VALUE(interface->command);
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);
  uint32_t waitStatus = 0;

  if (flags & SDIO_INITIALIZE)
    waitStatus |= INT_CDONE;

  //FIXME Rewrite wait status generation
  if (flags & SDIO_DATA_MODE)
  {
    /* Reset FIFO */
    reg->CTRL |= CTRL_FIFO_RESET;
    while (reg->CTRL & CTRL_FIFO_RESET);

    /* Enable data-relative interrupts */
    waitStatus |= INT_DTO;
    waitStatus |= INT_DCRC | INT_DRTO | INT_HTO | INT_FRUN;
  }

  if (!(flags & SDIO_DATA_MODE) && response != SDIO_RESPONSE_NONE)
    waitStatus |= INT_CDONE;

  waitStatus |= INT_EBE | INT_SBE | INT_HLE | INT_RTO | INT_RCRC | INT_RE;

  /* Initialize interrupts */
  reg->RINTSTS = INT_MASK;
  irqClearPending(interface->parent.irq);
  reg->INTMASK = waitStatus;
  irqEnable(interface->parent.irq);

  /* Prepare command */
  uint32_t command = code;

  if (flags & SDIO_INITIALIZE)
    command |= CMD_SEND_INITIALIZATION;
  if (flags & SDIO_CHECK_CRC)
    command |= CMD_CHECK_RESPONSE_CRC;
  if (flags & SDIO_DATA_MODE)
    command |= CMD_DATA_EXPECTED | CMD_WAIT_PRVDATA_COMPLETE;
  switch (response)
  {
    case SDIO_RESPONSE_LONG:
      command |= CMD_RESPONSE_LENGTH;
    case SDIO_RESPONSE_SHORT:
      command |= CMD_RESPONSE_EXPECT;
      break;

    default:
      break;
  }
  if (flags & SDIO_WRITE_MODE)
    command |= CMD_READ_WRITE;
  if (flags & SDIO_STREAM_MODE) /* FIXME Unused mode */
    command |= CMD_TRANSFER_MODE;
  if (flags & SDIO_AUTO_STOP)
    command |= CMD_SEND_AUTO_STOP;
  if (flags & SDIO_WAIT_DATA)
    command |= CMD_WAIT_PRVDATA_COMPLETE;

  /* Execute low-level command */
  interface->status = E_BUSY;
  if (executeCommand(interface, command, interface->argument) != E_OK)
    interface->status = E_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum result executeCommand(struct Sdmmc *interface, uint32_t command,
    uint32_t argument)
{
  LPC_SDMMC_Type * const reg = interface->parent.reg;

  /* TODO Add timeout */
  reg->CMDARG = argument;
  reg->CMD = command | CMD_START;

  /* Poll until command is accepted by the CIU */
  while (reg->CMD & CMD_START);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result updateRate(struct Sdmmc *interface, uint32_t rate)
{
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  const uint32_t clock = sdmmcGetClock((struct SdmmcBase *)interface);

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

  /* Enable clock */
  reg->CLKENA = CLKENA_CCLK_ENABLE;
  executeCommand(interface, CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  interface->rate = rate;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  const uint32_t hostErrors = INT_FRUN | INT_HLE;
  const uint32_t integrityErrors = INT_RE | INT_RCRC | INT_DCRC
      | INT_SBE | INT_EBE;
  const uint32_t timeoutErrors = INT_RTO | INT_DRTO | INT_HTO;

  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  const uint32_t status = reg->MINTSTS;

  pinSet(interface->debug); //FIXME Remove

  intSetEnabled(interface->finalizer, false);

  irqDisable(interface->parent.irq);
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
  else
  {
    if (reg->STATUS & STATUS_DATA_BUSY)
    {
      interface->status = E_BUSY;
      intSetEnabled(interface->finalizer, true);
      /* TODO Check whether operation is already completed */
    }
    else
      interface->status = E_OK;
  }

  if (interface->callback)
    interface->callback(interface->callbackArgument);

  pinReset(interface->debug); //FIXME Remove
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcConfig * const config = configBase;
  const struct DmaSdmmcConfig dmaConfig = {
      .burst = DMA_BURST_4,
      .number = 16,
      .parent = object
  };
  const struct PinInterruptConfig finalizerConfig = {
      .pin = config->dat0,
      .event = PIN_RISING,
      .pull = PIN_NOPULL
  };
  const struct SdmmcBaseConfig parentConfig = {
      .clk = config->clk,
      .cmd = config->cmd,
      .dat0 = config->dat0,
      .dat1 = config->dat1,
      .dat2 = config->dat2,
      .dat3 = config->dat3
  };
  struct Sdmmc * const interface = object;
  enum result res;

  if (!config->rate)
    return E_VALUE;

  interface->finalizer = init(PinInterrupt, &finalizerConfig);
  if (!interface->finalizer)
    return E_ERROR;
  intSetEnabled(interface->finalizer, false);
  intCallback(interface->finalizer, interruptHandler, interface);

  /* Call base class constructor */
  if ((res = SdmmcBase->init(object, &parentConfig)) != E_OK)
    return res;

  interface->debug = pinInit(PIN(PORT_1, 11)); //FIXME
  pinOutput(interface->debug, 0); //FIXME

  interface->parent.handler = interruptHandler;
  interface->argument = 0;
  interface->callback = 0;
  interface->command = 0;
  interface->status = E_OK;

  LPC_SDMMC_Type * const reg = interface->parent.reg;

  /* Reset specified blocks */
  reg->CTRL = CTRL_CONTROLLER_RESET;
  while (reg->CTRL & CTRL_CONTROLLER_RESET);

  /* Enable interrupts */
  reg->INTMASK = 0;
  reg->CTRL = CTRL_INT_ENABLE;
  /* Clear pending interrupts */
  reg->RINTSTS = INT_MASK;

  reg->TMOUT = 0xFFFFFFFF; //TODO Add timeout calculation

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
  LPC_SDMMC_Type * const reg = interface->parent.reg;

  deinit(interface->dma);
  reg->CTRL &= ~CTRL_INT_ENABLE;

  SdmmcBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static enum result sdioCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Sdmmc * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum result sdioGet(void *object, enum ifOption option, void *data)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;

  /* Additional options */
  switch ((enum sdioOption)option)
  {
    case IF_SDIO_MODE:
    {
      *(uint32_t *)data = interface->parent.wide ? SDIO_4BIT : SDIO_1BIT;
      return E_OK;
    }

    case IF_SDIO_RESPONSE:
    {
      const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);
      uint32_t *buffer = data;

      if (response == SDIO_RESPONSE_LONG)
      {
        for (uint8_t index = 0; index < 4; ++index)
          buffer[index] = reg->RESP[index];
      }
      else if (response == SDIO_RESPONSE_SHORT)
      {
        buffer[0] = reg->RESP[0];
      }
      else
        return E_ERROR;

      return E_OK;
    }

    default:
      break;
  }

  switch (option)
  {
    case IF_RATE:
      *(uint32_t *)data = interface->rate;
      return E_OK;

    case IF_STATUS:
      return interface->status;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result sdioSet(void *object, enum ifOption option,
    const void *data)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;

  /* Additional options */
  switch ((enum sdioOption)option)
  {
    case IF_SDIO_EXECUTE:
      execute(interface);
      return E_OK;

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

  switch (option)
  {
    case IF_RATE:
      return updateRate(interface, *(const uint32_t *)data);

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  enum result res;

  reg->BYTCNT = length;
  if ((res = dmaStart(interface->dma, buffer, 0, length)) != E_OK)
  {
    interface->status = res;
    return 0;
  }

  execute(interface);
  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  enum result res;

  reg->BYTCNT = length;
  if ((res = dmaStart(interface->dma, 0, buffer, length)) != E_OK)
  {
    interface->status = res;
    return 0;
  }

  execute(interface);
  return length;
}
