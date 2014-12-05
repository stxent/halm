/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <modules/sdio.h>
#include <modules/sdio_defs.h>
#include <platform/platform_defs.h>
#include <platform/nxp/sdmmc.h>
#include <platform/nxp/sdmmc_defs.h>
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
#define FIFOSZ 32 //FIXME
/*----------------------------------------------------------------------------*/
static void execute(struct Sdmmc *interface)
{
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  const uint32_t code = COMMAND_CODE_VALUE(interface->command);
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);
  uint32_t waitStatus = 0;

  if (flags & SDIO_INITIALIZE)
    waitStatus |= INTMASK_CDONE;

  //FIXME Rewrite wait status generation
  if (flags & SDIO_DATA_MODE)
  {
    /* Reset FIFO */
    reg->CTRL |= CTRL_FIFO_RESET;
    while (reg->CTRL & CTRL_FIFO_RESET);

    /* Enable data-relative interrupts */
    waitStatus |= INTMASK_DTO;
    waitStatus |= INTMASK_DCRC | INTMASK_DRTO | INTMASK_HTO | INTMASK_FRUN;
  }

  if (!(flags & SDIO_DATA_MODE) && response != SDIO_RESPONSE_NONE)
    waitStatus |= INTMASK_CDONE;

  waitStatus |= INTMASK_EBE | INTMASK_SBE | INTMASK_HLE |
      INTMASK_RTO | INTMASK_RCRC | INTMASK_RE;

  /* Initialize interrupts */
  reg->RINTSTS = 0xFFFFFFFF; /* Clear pending interrupts */
  irqClearPending(interface->parent.irq);
  reg->INTMASK = waitStatus;
  irqEnable(interface->parent.irq);

  /* Prepare command */
  uint32_t command = code;

  if (flags & SDIO_INITIALIZE)
    command |= CMD_SEND_INITIALIZATION;
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
  /* TODO Stop/Abort command */

  /* Execute low-level command */
  interface->status = E_BUSY;
  if (executeCommand(interface, command, interface->argument) != E_OK)
    interface->status = E_VALUE;
}
/*----------------------------------------------------------------------------*/
static enum result executeCommand(struct Sdmmc *interface,
    uint32_t command, uint32_t argument)
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

  /* Disable clock and reset set user divider */
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
  const uint32_t hostErrors = INTMASK_FRUN | INTMASK_HLE;
  const uint32_t integrityErrors = INTMASK_RE | INTMASK_RCRC | INTMASK_DCRC
      | INTMASK_SBE | INTMASK_EBE;
  const uint32_t timeoutErrors = INTMASK_RTO | INTMASK_DRTO | INTMASK_HTO;

  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;
  const uint32_t status = reg->MINTSTS;

  pinSet(interface->debug); //FIXME Remove

  irqDisable(interface->parent.irq);
  reg->INTMASK = 0;
  reg->RINTSTS = 0xFFFFFFFF; //TODO Add define

  if (status & hostErrors)
    interface->status = E_ERROR;
  else if (status & integrityErrors)
    interface->status = E_INTERFACE;
  else if (status & timeoutErrors)
    interface->status = E_TIMEOUT;
  else
    interface->status = E_OK;

  if (interface->callback)
    interface->callback(interface->callbackArgument);

  pinReset(interface->debug); //FIXME Remove
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcConfig * const config = configBase;
  const struct SdmmcBaseConfig parentConfig = {
      .clock = config->clock,
      .cmd = config->cmd,
      .dat0 = config->dat0,
      .dat1 = config->dat1,
      .dat2 = config->dat2,
      .dat3 = config->dat3
  };
  struct Sdmmc * const interface = object;
  enum result res;

  /* Call base class constructor */
  if ((res = SdmmcBase->init(object, &parentConfig)) != E_OK)
    return res;

  interface->debug = pinInit(PIN(PORT_5, 2));
  pinOutput(interface->debug, 0);

  interface->parent.handler = interruptHandler;
  interface->argument = 0;
  interface->callback = 0;
  interface->command = 0;
  interface->rate = 400000; //FIXME
  interface->status = E_OK;

  LPC_SDMMC_Type * const reg = interface->parent.reg;

  /* Reset DMA controller */
  reg->BMOD = BMOD_SWR;

  /* Reset specified blocks */
  const uint32_t resetMask = CTRL_CONTROLLER_RESET | CTRL_FIFO_RESET
      | CTRL_DMA_RESET;

  reg->CTRL = resetMask;
  while (reg->CTRL & resetMask);

  /* Internal DMA setup */
  reg->CTRL = CTRL_USE_INTERNAL_DMAC | CTRL_INT_ENABLE;

  /* Clear pending interrupts */
  reg->INTMASK = 0;
  reg->RINTSTS = 0xFFFFFFFF;

  reg->FIFOTH = FIFOTH_TX_WMARK(FIFOSZ / 2) | FIFOTH_RX_WMARK(FIFOSZ / 2 - 1)
      | FIFOTH_DMA_MTS(BURST_SIZE_4);
  reg->TMOUT = 0xFFFFFFFF;

  /* Set default block length */
  reg->BLKSIZ = 512;
  /* Enable internal DMA */
  reg->BMOD = BMOD_DE | BMOD_DSL(4) | BMOD_PBL(BURST_SIZE_4);

  reg->CLKENA = 0;
  reg->CLKSRC = 0;

  /* Set default clock rate */
  updateRate(interface, interface->rate); //TODO Rewrite

  //FIXME
  /* Enable power */
//  LPC_SDMMC->PWREN = 1;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  SdmmcBase->deinit(object);
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
      *(uint32_t *)data = SDIO_1BIT; //TODO
      return E_OK;
    }

    case IF_SDIO_RESPONSE:
    {
      const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);
      uint32_t *buffer = data;

      if (response == SDIO_RESPONSE_NONE)
        return E_ERROR;

      if (response == SDIO_RESPONSE_SHORT)
      {
        buffer[0] = reg->RESP0;
      }
      else
      {
        /* TODO Rewrite */
        buffer[0] = reg->RESP0;
        buffer[1] = reg->RESP1;
        buffer[2] = reg->RESP2;
        buffer[3] = reg->RESP3;
      }
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

    case IF_SDIO_BLOCK_LENGTH:
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
void dmaSetup(struct Sdmmc *interface, uint8_t *buffer, uint32_t length)
{
  //TODO Move to separate DmaList-based class
  LPC_SDMMC_Type * const reg = interface->parent.reg;

  int i = 0;
  uint32_t ctrl, maxs;

  /* Reset DMA */
  reg->CTRL |= CTRL_DMA_RESET | CTRL_FIFO_RESET;
  while (reg->CTRL & CTRL_DMA_RESET);

  /* Build a descriptor list using the chained DMA method */
  while (length > 0)
  {
    /* Limit size of the transfer to maximum buffer size */
    maxs = length <= DESC_SIZE_MAX ? length : DESC_SIZE_MAX;
    length -= maxs;

    /* Set buffer size */
    interface->descriptor[i].size = DESC_SIZE_BS1(maxs);

    /* Setup buffer address (chained) */
    interface->descriptor[i].buffer1 = (uint32_t)(buffer + (i * DESC_SIZE_MAX));

    /* Setup basic control */
    ctrl = DESC_CONTROL_OWN | DESC_CONTROL_CH;
    if (i == 0)
      ctrl |= DESC_CONTROL_FS; /* First DMA buffer */

    /* No more data? Then this is the last descriptor */
    if (!length)
      ctrl |= DESC_CONTROL_LD;
    else
      ctrl |= DESC_CONTROL_DIC;

    /* Another descriptor is needed */
    interface->descriptor[i].buffer2 =
        (uint32_t)(&interface->descriptor[i + 1]);
    interface->descriptor[i].control = ctrl;

    ++i;
  }

  /* Set DMA descriptor base address */
  reg->DBADDR = (uint32_t)&interface->descriptor[0];
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->parent.reg;

  reg->BYTCNT = length;
  dmaSetup(interface, buffer, length);

  execute(interface);

  return length;
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Sdmmc * const interface = object;

  return 0;
}
