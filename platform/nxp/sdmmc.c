/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <delay.h> //TODO Remove
#include <modules/sdio.h>
#include <modules/sdio_defs.h>
#include <platform/platform_defs.h>
#include <platform/nxp/sdmmc.h>
#include <platform/nxp/sdmmc_defs.h>
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
#define OCR_VOLTAGE_RANGE_MSK   0x00FF8000
/*----------------------------------------------------------------------------*/
bool sendLowLevelCommand(uint32_t cmd, uint32_t arg)
{
  int32_t tmo = 50;

  /* set command arg reg*/
  LPC_SDMMC->CMDARG = arg;
  LPC_SDMMC->CMD = CMD_START | cmd;

  /* Poll until command is accepted by the CIU */
  while (--tmo && (LPC_SDMMC->CMD & CMD_START))
    udelay(100);

  return tmo < 1;
}
/*----------------------------------------------------------------------------*/
void sdmmcSetClock(uint32_t speed)
{
  /* compute SD/MMC clock dividers */
  uint32_t div;

  //FIXME Argument value
  div = ((sdmmcGetClock(0) / speed) + 2) >> 1;

  if ((div == LPC_SDMMC->CLKDIV) && LPC_SDMMC->CLKENA)
    return; /* Closest speed is already set */

  /* disable clock */
  LPC_SDMMC->CLKENA = 0;

  /* User divider 0 */
  LPC_SDMMC->CLKSRC = CLKSRC_CLK_SOURCE(0);

  /* inform CIU */
  sendLowLevelCommand(CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  /* Set divider 0 to desired value */
  LPC_SDMMC->CLKDIV = (LPC_SDMMC->CLKDIV & CLKDIV_CLK_DIVIDER_MASK(0))
      | CLKDIV_CLK_DIVIDER(0, div);

  /* inform CIU */
  sendLowLevelCommand(CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  /* enable clock */
  LPC_SDMMC->CLKENA = CLKENA_CCLK_ENABLE;

  /* inform CIU */
  sendLowLevelCommand(CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);
}
/*----------------------------------------------------------------------------*/
static bool waitExit = false;
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *object)
{
  struct Sdmmc * const interface = object;

  pinSet(interface->debug);
  irqDisable(interface->parent.irq);
  waitExit = true; //FIXME
  pinReset(interface->debug);

  //FIXME Add callback
}
/*----------------------------------------------------------------------------*/
void evSetup(uint32_t bits)
{
  irqClearPending(SDIO_IRQ);
  waitExit = false;
  LPC_SDMMC->INTMASK = bits;
  irqEnable(SDIO_IRQ);
}
/*----------------------------------------------------------------------------*/
static uint32_t waitCallback(uint32_t bits)
{
  uint32_t status;

  while (waitExit == 0)
    barrier();

  /* Get status and clear interrupts */
  status = LPC_SDMMC->RINTSTS;
  LPC_SDMMC->RINTSTS = status;
  LPC_SDMMC->INTMASK = 0;

  return status;
}
/*----------------------------------------------------------------------------*/
#define SD_INT_ERROR (INTMASK_RE | INTMASK_RCRC | INTMASK_DCRC | \
    INTMASK_RTO | INTMASK_DTO | INTMASK_HTO | INTMASK_FRUN | INTMASK_HLE | \
    INTMASK_SBE | INTMASK_EBE)
/*----------------------------------------------------------------------------*/
static enum result execute(struct Sdmmc *interface)
{
  const uint32_t code = COMMAND_CODE_VALUE(interface->command);
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  const enum sdioResponse response = COMMAND_RESP_VALUE(interface->command);
  uint32_t waitStatus = 0;

  if (flags & SDIO_INITIALIZE)
    waitStatus |= INTMASK_CDONE;

  if (response != SDIO_RESPONSE_NONE)
    waitStatus |= INTMASK_CDONE;
  else
    waitStatus |= INTMASK_DTO;

  if (flags & SDIO_DATA_MODE)
  {
    /* Reset FIFO */
    LPC_SDMMC->CTRL |= CTRL_FIFO_RESET;
    while (LPC_SDMMC->CTRL & CTRL_FIFO_RESET);

    /* Clear interrupt status */
    LPC_SDMMC->RINTSTS = 0xFFFFFFFF;
  }

  waitStatus |= INTMASK_EBE | INTMASK_SBE | INTMASK_HLE |
      INTMASK_RTO | INTMASK_RCRC | INTMASK_RE;
  if (waitStatus & INTMASK_DTO)
    waitStatus |= INTMASK_FRUN | INTMASK_HTO | INTMASK_DTO | INTMASK_DCRC;

  /* Clear the interrupts */
  //FIXME Redundant
  LPC_SDMMC->RINTSTS = 0xFFFFFFFF;
  evSetup(waitStatus);

  uint32_t command = code | CMD_START;

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
  /* TODO Stop/Abort command */
  /* TODO Select/Deselect command */

  /* Wait for command to be accepted by CIU */
  if (sendLowLevelCommand(command, interface->argument) != 0)
  {
    interface->status = E_ERROR;
    return interface->status;
  }

  /* Wait for command response */
  const uint32_t status = waitCallback(waitStatus); //TODO Rewrite

  //TODO Check INTMASK_RTO for command availability
  //TODO Rewrite
  /* Return an error if there is a timeout */
  if (status & SD_INT_ERROR)
  {
    interface->status = E_TIMEOUT;
  }
  else
  {
    interface->status = E_OK;
  }

  return interface->status;
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
  interface->command = 0;
  interface->rate = 400000; //FIXME
  interface->status = E_OK;

  /* Software reset */
  LPC_SDMMC->BMOD = BMOD_SWR;

  /* Reset all blocks */
  const uint32_t resetMask = CTRL_CONTROLLER_RESET | CTRL_FIFO_RESET
      | CTRL_DMA_RESET;

  LPC_SDMMC->CTRL = resetMask;
  while (LPC_SDMMC->CTRL & resetMask);

  /* Internal DMA setup */
  LPC_SDMMC->CTRL = CTRL_USE_INTERNAL_DMAC | CTRL_INT_ENABLE;
  LPC_SDMMC->INTMASK = 0;

  LPC_SDMMC->RINTSTS = 0xFFFFFFFF;
  LPC_SDMMC->TMOUT = 0xFFFFFFFF;

  LPC_SDMMC->FIFOTH = FIFOTH_TX_WMARK(FIFOSZ / 2)
      | FIFOTH_RX_WMARK(FIFOSZ / 2 - 1) | FIFOTH_DMA_MTS(BURST_SIZE_4);

  /* Enable internal DMA */
  LPC_SDMMC->BMOD = BMOD_DE | BMOD_DSL(4) | BMOD_PBL(BURST_SIZE_4);

  LPC_SDMMC->CLKENA = 0;
  LPC_SDMMC->CLKSRC = 0;

  sdmmcSetClock(400000);
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

  /* Additional options */
  switch ((enum sdioOption)option)
  {
    case IF_SDIO_EXECUTE:
      execute(interface);
      return E_OK;

    case IF_SDIO_ARGUMENT:
      interface->argument = *(const uint32_t *)data;
      return E_OK;

    case IF_SDIO_COMMAND:
      interface->command = *(const uint32_t *)data;
      return E_OK;

    default:
      break;
  }

  switch (option)
  {
    case IF_RATE:
      /* TODO */
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct Sdmmc * const interface = object;

  return 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct Sdmmc * const interface = object;

  return 0;
}
