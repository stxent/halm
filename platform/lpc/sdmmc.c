/*
 * sdmmc.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/sdio.h>
#include <halm/generic/sdio_defs.h>
#include <halm/platform/lpc/dma_sdmmc.h>
#include <halm/platform/lpc/pin_int.h>
#include <halm/platform/lpc/sdmmc.h>
#include <halm/platform/lpc/sdmmc_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_BLOCK_SIZE  512
#define BUSY_READ_DELAY     100 /* Milliseconds */
#define BUSY_WRITE_DELAY    500 /* Milliseconds */
/*----------------------------------------------------------------------------*/
static void execute(struct Sdmmc *);
static void executeCommand(struct Sdmmc *, uint32_t, uint32_t);
static void pinInterruptHandler(void *);
static void sdioInterruptHandler(void *);
static enum Result updateRate(struct Sdmmc *, uint32_t);

#ifdef CONFIG_PLATFORM_LPC_SDMMC_PM
static void powerStateHandler(void *, enum PmState);
#endif
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static void sdioSetCallback(void *, void (*)(void *), void *);
static enum Result sdioGetParam(void *, int, void *);
static enum Result sdioSetParam(void *, int, const void *);
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
  const enum SDIOResponse response = COMMAND_RESP_VALUE(interface->command);

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
  reg->INTMASK = waitStatus;

  /* Prepare command */
  uint32_t command = CMD_INDEX(code);

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

  interface->status = E_BUSY;

  /* Enable interrupts */
  irqClearPending(interface->base.irq);
  irqEnable(interface->base.irq);

  /* Execute low-level command */
  executeCommand(interface, command, interface->argument);
}
/*----------------------------------------------------------------------------*/
static void executeCommand(struct Sdmmc *interface, uint32_t command,
    uint32_t argument)
{
  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* TODO Add timeout */
  reg->CMDARG = argument;
  reg->CMD = command | CMD_START;

  /* Poll until command is accepted by the CIU */
  while (reg->CMD & CMD_START);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptHandler(void *object)
{
  struct Sdmmc * const interface = object;

  /* Disable further DATA0 interrupts */
  interruptDisable(interface->finalizer);

  interface->status = E_OK;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void sdioInterruptHandler(void *object)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;
  const uint32_t intStatus = reg->MINTSTS;

  /* Disable further DATA0 interrupts */
  interruptDisable(interface->finalizer);
  /* Clear pending SDMMC interrupts */
  reg->RINTSTS = intStatus;

  if (intStatus & (INT_FRUN | INT_HLE))
  {
    /* Host error */
    interface->status = E_ERROR;
  }
  else if (intStatus & (INT_RE | INT_RCRC | INT_DCRC | INT_SBE | INT_EBE))
  {
    /* Integrity error */
    interface->status = E_INTERFACE;
  }
  else if (intStatus & (INT_RTO | INT_DRTO | INT_HTO))
  {
    /* Timeout error */
    interface->status = E_TIMEOUT;
  }
  else
  {
    const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
    bool completed;

    if (flags & SDIO_DATA_MODE)
    {
      completed = (intStatus & INT_DTO) != 0;
    }
    else
    {
      completed = (intStatus & INT_CDONE) != 0;
    }

    if (completed)
    {
      if (!pinRead(interface->data0))
      {
        interruptEnable(interface->finalizer);

        if (pinRead(interface->data0))
        {
          interruptDisable(interface->finalizer);
          interface->status = E_OK;
        }
        else
        {
          /* Disable SDMMC interrupts, wait for high level on DATA0 */
          irqDisable(interface->base.irq);
        }
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
static enum Result updateRate(struct Sdmmc *interface, uint32_t rate)
{
  LPC_SDMMC_Type * const reg = interface->base.reg;
  const uint32_t clock = sdmmcGetClock(&interface->base);

  if (rate > clock)
    return E_VALUE;

  const uint32_t current = CLKDIV_VALUE(0, reg->CLKDIV);
  const uint32_t divider = ((clock + (rate - 1)) / rate) >> 1;

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
#ifdef CONFIG_PLATFORM_LPC_SDMMC_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct Sdmmc * const interface = object;
    updateRate(interface, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rate);

  const struct DmaSdmmcConfig dmaConfig = {
      .burst = DMA_BURST_4,
      .number = 16,
      .parent = object
  };
  const struct PinIntConfig finalizerConfig = {
      .pin = config->dat0,
      .priority = config->priority,
      .event = INPUT_RISING,
      .pull = PIN_NOPULL
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

  interface->data0 = pinInit(config->dat0);
  assert(pinValid(interface->data0));

  interface->finalizer = init(PinInt, &finalizerConfig);
  if (interface->finalizer == NULL)
    return E_ERROR;
  interruptSetCallback(interface->finalizer, pinInterruptHandler, interface);

  /* Call base class constructor */
  if ((res = SdmmcBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = sdioInterruptHandler;
  interface->argument = 0;
  interface->callback = NULL;
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

#ifdef CONFIG_PLATFORM_LPC_SDMMC_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  /* Internal DMA controller should be initialized after interface setup */
  interface->dma = init(DmaSdmmc, &dmaConfig);
  if (interface->dma == NULL)
    return E_ERROR;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* Disable interrupts */
  reg->CTRL &= ~CTRL_INT_ENABLE;

#ifdef CONFIG_PLATFORM_LPC_SDMMC_PM
  pmUnregister(interface);
#endif

  deinit(interface->dma);
  deinit(interface->finalizer);
  SdmmcBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static void sdioSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Sdmmc * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioGetParam(void *object, int parameter, void *data)
{
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

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
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  /* Additional options */
  switch ((enum SDIOParameter)parameter)
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
  struct Sdmmc * const interface = object;
  LPC_SDMMC_Type * const reg = interface->base.reg;

  reg->BYTCNT = length;
  dmaAppend(interface->dma, buffer, NULL, length);

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
  dmaAppend(interface->dma, NULL, buffer, length);

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
