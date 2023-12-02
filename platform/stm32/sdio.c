/*
 * sdio.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/dma.h>
#include <halm/generic/sdio.h>
#include <halm/generic/sdio_defs.h>
#include <halm/platform/stm32/exti.h>
#include <halm/platform/stm32/sdio.h>
#include <halm/platform/stm32/sdio_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/pm.h>
#include <halm/timer.h>
#include <xcore/accel.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_BLOCK_SIZE    9   /* 512 bytes */
#define BUSY_READ_DELAY       100 /* Milliseconds */
#define BUSY_WRITE_DELAY      500 /* Milliseconds */
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Sdio *, uint8_t);
static void execute(struct Sdio *, uint32_t, uint32_t);
static void dmaInterruptHandler(void *);
static void pinInterruptHandler(void *);
static void sdioInterruptHandler(void *);
static void timerInterruptHandler(void *);
static enum Result updateRate(struct Sdio *, uint32_t);

#ifdef CONFIG_PLATFORM_STM32_SDIO_PM
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
const struct InterfaceClass * const Sdio = &(const struct InterfaceClass){
    .size = sizeof(struct Sdio),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .setCallback = sdioSetCallback,
    .getParam = sdioGetParam,
    .setParam = sdioSetParam,
    .read = sdioRead,
    .write = sdioWrite
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Sdio *interface, uint8_t stream)
{
  static const struct DmaSettings rxDmaSettings = {
      .source = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_WORD,
          .increment = false
      },
      .destination = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_WORD,
          .increment = true
      }
  };
  static const struct DmaSettings txDmaSettings = {
      .source = {
          .burst = DMA_BURST_4,
          .width = DMA_WIDTH_WORD,
          .increment = true
      },
      .destination = {
          .burst = DMA_BURST_1,
          .width = DMA_WIDTH_WORD,
          .increment = false
      }
  };

  interface->rxDma = sdioMakeOneShotDma(stream, DMA_PRIORITY_LOW, DMA_TYPE_P2M);
  if (interface->rxDma == NULL)
    return false;
  dmaConfigure(interface->rxDma, &rxDmaSettings);
  dmaSetCallback(interface->rxDma, dmaInterruptHandler, interface);

  interface->txDma = sdioMakeOneShotDma(stream, DMA_PRIORITY_LOW, DMA_TYPE_M2P);
  if (interface->txDma == NULL)
    return false;
  dmaConfigure(interface->txDma, &txDmaSettings);
  dmaSetCallback(interface->txDma, dmaInterruptHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static void execute(struct Sdio *interface, uint32_t command, uint32_t argument)
{
  STM_SDIO_Type * const reg = interface->base.reg;
  const uint32_t code = COMMAND_CODE_VALUE(command);
  const uint16_t flags = COMMAND_FLAG_VALUE(command);
  const enum SDIOResponse response = COMMAND_RESP_VALUE(command);

  /* Prepare interrupt mask */
  uint32_t mask = 0;

  if (flags & SDIO_DATA_MODE)
  {
    /* Enable data-relative interrupts */
    mask |= MASK_DTIMEOUTIE | MASK_DATAENDIE;

    if (flags & SDIO_CHECK_CRC)
      mask |= MASK_DCRCFAILIE;

    if (flags & SDIO_WRITE_MODE)
      mask |= MASK_TXUNDERRIE;
    else
      mask |= MASK_RXOVERRIE | MASK_STBITERRIE;
  }

  if (response == SDIO_RESPONSE_NONE)
    mask |= MASK_CMDSENTIE;
  else
    mask |= MASK_CCRCFAILIE | MASK_CTIMEOUTIE | MASK_CMDRENDIE;

  if (flags & SDIO_CHECK_CRC)
    mask |= MASK_CCRCFAILIE;

  /* Prepare command */
  uint32_t cmd = CMD_CMDINDEX(code) | CMD_CPSMEN;

  switch (response)
  {
    case SDIO_RESPONSE_LONG:
      cmd |= CMD_WAITRESP(WAITRESP_LONG);
      break;

    case SDIO_RESPONSE_SHORT:
      cmd |= CMD_WAITRESP(WAITRESP_SHORT);
      break;

    default:
      break;
  }

  interface->cmdStatus = E_BUSY;
  interface->dmaStatus = E_OK;

  /* Initialize interrupts */
  reg->ICR = ICR_MASK;
  reg->MASK = mask;

  /* Execute low-level command */
  reg->DTIMER = 0;
  reg->DLEN = 0;
  reg->DCTRL = 0;
  reg->ARG = argument;
  reg->CMD = cmd;
}
/*----------------------------------------------------------------------------*/
static void dmaInterruptHandler(void *object)
{
  struct Sdio * const interface = object;
  STM_SDIO_Type * const reg = interface->base.reg;
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  bool completed = false;

  if (flags & SDIO_WRITE_MODE)
  {
    /* Disable possible SDIO interrupts and wait for high level on DAT0 */
    reg->MASK = 0;

    interface->dmaStatus = dmaStatus(interface->txDma);
    completed = true;
  }
  else
  {
    /* Wait for SDIO interrupts */
    interface->dmaStatus = dmaStatus(interface->rxDma);

    if (!reg->MASK)
      completed = true;
  }

  if (completed)
  {
    /* Wait for high level on DATA0 */
    if (!pinRead(interface->data0))
    {
      if (interface->timer != NULL)
      {
        timerSetValue(interface->timer, 0);
        timerEnable(interface->timer);
      }
      interruptEnable(interface->finalizer);

      if (pinRead(interface->data0))
      {
        interruptDisable(interface->finalizer);
        if (interface->timer != NULL)
          timerDisable(interface->timer);
        interface->cmdStatus = interface->dmaStatus;
      }
    }
    else
      interface->cmdStatus = interface->dmaStatus;
  }

  if (interface->cmdStatus != E_BUSY && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void pinInterruptHandler(void *object)
{
  struct Sdio * const interface = object;

  /* Disable further DATA0 interrupts */
  interruptDisable(interface->finalizer);

  interface->cmdStatus = interface->dmaStatus;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void sdioInterruptHandler(void *object)
{
  struct Sdio * const interface = object;
  STM_SDIO_Type * const reg = interface->base.reg;
  const uint16_t flags = COMMAND_FLAG_VALUE(interface->command);
  const uint32_t intStatus = reg->STA;

  /* Disable further DATA0 interrupts */
  interruptDisable(interface->finalizer);
  /* Clear pending SDIO interrupts */
  reg->ICR = intStatus;

  if (intStatus & (STA_TXUNDERR | STA_RXOVERR))
  {
    /* Host error */
    interface->cmdStatus = E_ERROR;
  }
  else if (intStatus & (STA_DCRCFAIL | STA_STBITERR))
  {
    /* Integrity error */
    interface->cmdStatus = E_INTERFACE;
  }
  else if (intStatus & (STA_CTIMEOUT | STA_DTIMEOUT))
  {
    /* Timeout error */
    interface->cmdStatus = E_TIMEOUT;
  }
  else if (intStatus & STA_CCRCFAIL)
  {
    if (!(flags & SDIO_CHECK_CRC))
    {
      /* Response does not contain CRC field, treat CRC failure as success */
      interface->cmdStatus = E_OK;
    }
    else
    {
      /* Integrity error */
      interface->cmdStatus = E_INTERFACE;
    }
  }
  else
  {
    bool completed = false;

    if (intStatus & STA_DATAEND)
    {
      /* Data end interrupt */
      if (interface->dmaStatus != E_BUSY)
      {
        /* DMA transfer is already completed, terminate the command execution */
        completed = true;
      }
      else
      {
        /* Disable further SDIO interrupts, wait for DMA callback */
        reg->MASK = 0;
      }
    }
    else
    {
      /* Command sent or response received interrupts */
      if (flags & SDIO_DATA_MODE)
      {
        /* Start data transfer phase */
        uint32_t dataControl = DCTRL_DBLOCKSIZE(interface->block);
        enum Result res;

        if (!(flags & SDIO_WRITE_MODE))
          dataControl |= DCTRL_DTDIR;
        dataControl |= DCTRL_DMAEN | DCTRL_DTEN | DCTRL_SDIOEN;

        reg->DTIMER = interface->timeout;
        reg->DLEN = interface->count;
        reg->DCTRL = dataControl;

        if (flags & SDIO_WRITE_MODE)
          res = dmaEnable(interface->txDma);
        else
          res = dmaEnable(interface->rxDma);

        if (res != E_OK)
        {
          interface->dmaStatus = res;
          completed = true;
        }
      }
      else
        completed = true;
    }

    if (completed)
    {
      /* Wait for high level on DAT0 after a completed command */
      if (!pinRead(interface->data0))
      {
        if (interface->timer != NULL)
        {
          timerSetValue(interface->timer, 0);
          timerEnable(interface->timer);
        }
        interruptEnable(interface->finalizer);

        if (pinRead(interface->data0))
        {
          interruptDisable(interface->finalizer);
          if (interface->timer != NULL)
            timerDisable(interface->timer);
          interface->cmdStatus = interface->dmaStatus;
        }
      }
      else
        interface->cmdStatus = interface->dmaStatus;
    }
  }

  if (interface->cmdStatus != E_BUSY)
  {
    /* Disable SDIO interrupts */
    reg->MASK = 0;

    if (interface->callback != NULL)
      interface->callback(interface->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static void timerInterruptHandler(void *argument)
{
  struct Sdio * const interface = argument;

  /* Disable further DATA0 interrupts */
  interruptDisable(interface->finalizer);

  interface->cmdStatus = E_TIMEOUT;

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result updateRate(struct Sdio *interface, uint32_t rate)
{
  STM_SDIO_Type * const reg = interface->base.reg;
  const uint32_t clock = sdioGetClock(&interface->base);

  if (rate >= clock)
  {
    /* Enable bypass mode, clear divider */
    reg->CLKCR = (reg->CLKCR & ~CLKCR_CLKDIV_MASK) | CLKCR_BYPASS | CLKCR_CLKEN;
  }
  else
  {
    uint32_t divider = (clock + (rate - 1)) / rate - 2;

    if (divider > CLKCR_CLKDIV_MAX)
      divider = CLKCR_CLKDIV_MAX;

    reg->CLKCR = (reg->CLKCR & ~(CLKCR_CLKDIV_MASK | CLKCR_BYPASS))
        | CLKCR_CLKDIV(divider) | CLKCR_CLKEN;
  }

  interface->timeout = BUSY_READ_DELAY * (rate / 1000);
  interface->rate = rate;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_STM32_SDIO_PM
static void powerStateHandler(void *object, enum PmState state)
{
  if (state == PM_ACTIVE)
  {
    struct Sdio * const interface = object;
    updateRate(interface, interface->rate);
  }
}
#endif
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdioConfig * const config = configBase;
  assert(config != NULL);
  assert(config->rate);

  const struct ExtiConfig finalizerConfig = {
      .pin = config->dat0,
      .priority = config->priority,
      .event = INPUT_RISING,
      .pull = PIN_NOPULL
  };
  const struct SdioBaseConfig baseConfig = {
      .clk = config->clk,
      .cmd = config->cmd,
      .dat0 = config->dat0,
      .dat1 = config->dat1,
      .dat2 = config->dat2,
      .dat3 = config->dat3
  };
  struct Sdio * const interface = object;
  enum Result res;

  interface->data0 = pinInit(config->dat0);
  assert(pinValid(interface->data0));

  interface->finalizer = init(Exti, &finalizerConfig);
  if (interface->finalizer == NULL)
    return E_ERROR;
  interruptSetCallback(interface->finalizer, pinInterruptHandler, interface);

  /* Call base class constructor */
  if ((res = SdioBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!dmaSetup(interface, config->dma))
    return E_ERROR;

  interface->base.handler = sdioInterruptHandler;
  interface->timer = config->timer;
  interface->argument = 0;
  interface->callback = NULL;
  interface->command = 0;
  interface->count = 0;
  interface->cmdStatus = E_OK;
  interface->dmaStatus = E_OK;
  interface->block = DEFAULT_BLOCK_SIZE;

  STM_SDIO_Type * const reg = interface->base.reg;

  /* Disable card clock and power before initialization */
  reg->POWER = POWER_PWRCTRL(PWRCTRL_POWER_OFF);
  reg->CLKCR = 0;

  /* Clear and disable interrupts in the peripheral */
  reg->MASK = 0;
  reg->ICR = ICR_MASK;

  /* Clear data and command transfer settings */
  reg->CMD = 0;
  reg->ARG = 0;
  reg->DCTRL = 0;
  reg->DTIMER = 0;
  reg->DLEN = 0;

  /* Set bus width */
  reg->CLKCR = interface->base.width == 8 ?
      CLKCR_WIDBUS(WIDBUS_8BIT) : (interface->base.width == 4 ?
          CLKCR_WIDBUS(WIDBUS_4BIT) : CLKCR_WIDBUS(WIDBUS_1BIT));

  /* Set default clock rate and enable card clock */
  updateRate(interface, config->rate);

  /* Enable interrupts in the NVIC */
  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  /* Enable card power */
  reg->POWER = POWER_PWRCTRL(PWRCTRL_POWER_ON);

#ifdef CONFIG_PLATFORM_STM32_SDIO_PM
  if ((res = pmRegister(powerStateHandler, interface)) != E_OK)
    return res;
#endif

  if (interface->timer != NULL)
  {
    static const uint64_t timeout = BUSY_WRITE_DELAY * (1ULL << 32) / 1000;
    const uint32_t frequency = timerGetFrequency(interface->timer);
    const uint32_t overflow = (frequency * timeout + ((1ULL << 32) - 1)) >> 32;

    timerSetAutostop(interface->timer, true);
    timerSetCallback(interface->timer, timerInterruptHandler, interface);
    timerSetOverflow(interface->timer, overflow);
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object)
{
  struct Sdio * const interface = object;
  STM_SDIO_Type * const reg = interface->base.reg;

  /* Stop DMA channels */
  dmaDisable(interface->txDma);
  dmaDisable(interface->rxDma);

  /* Disable interrupts */
  irqDisable(interface->base.irq);

  /* Disable card power */
  reg->POWER = POWER_PWRCTRL(PWRCTRL_POWER_OFF);

#ifdef CONFIG_PLATFORM_STM32_SDIO_PM
  pmUnregister(interface);
#endif

  /* Free DMA channels */
  deinit(interface->txDma);
  deinit(interface->rxDma);

  deinit(interface->finalizer);
  SdioBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static void sdioSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Sdio * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result sdioGetParam(void *object, int parameter, void *data)
{
  struct Sdio * const interface = object;
  STM_SDIO_Type * const reg = interface->base.reg;

  /* Additional options */
  switch ((enum SDIOParameter)parameter)
  {
    case IF_SDIO_MODE:
    {
      uint8_t width;

      if (interface->base.width == 8)
        width = SDIO_8BIT;
      else if (interface->base.width == 4)
        width = SDIO_4BIT;
      else
        width = SDIO_1BIT;

      *(uint8_t *)data = width;
      return E_OK;
    }

    case IF_SDIO_RESPONSE:
    {
      const enum SDIOResponse response = COMMAND_RESP_VALUE(interface->command);
      uint32_t * const buffer = data;

      if (response == SDIO_RESPONSE_LONG)
      {
        for (size_t index = 0; index < 4; ++index)
          buffer[index] = reg->RESP[3 - index];

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
      return interface->cmdStatus;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result sdioSetParam(void *object, int parameter, const void *data)
{
  struct Sdio * const interface = object;

  /* Additional options */
  switch ((enum SDIOParameter)parameter)
  {
    case IF_SDIO_EXECUTE:
      execute(interface, interface->command, interface->argument);
      return E_BUSY;

    case IF_SDIO_ARGUMENT:
      interface->argument = *(const uint32_t *)data;
      return E_OK;

    case IF_SDIO_BLOCK_SIZE:
    {
      const uint32_t blockLength = *(const uint32_t *)data;

      if (blockLength)
      {
        const uint32_t blockPow = 31 - countLeadingZeros32(blockLength);

        if (blockPow <= DCTRL_DBLOCKSIZE_MAX && (1U << blockPow) == blockLength)
        {
          interface->block = blockPow;
          return E_OK;
        }
      }

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
  /* Buffer address must be aligned on a 4-byte boundary */
  assert((uintptr_t)buffer % 4 == 0);
  /* Buffer size must be aligned on a 4-byte boundary */
  assert(length % 4 == 0);

  struct Sdio * const interface = object;
  STM_SDIO_Type * const reg = interface->base.reg;

  interface->count = length;
  dmaAppend(interface->rxDma, buffer, (const void *)&reg->FIFO, length >> 2);

  execute(interface, interface->command, interface->argument);
  return length;
}
/*----------------------------------------------------------------------------*/
static size_t sdioWrite(void *object, const void *buffer, size_t length)
{
  /* Buffer address must be aligned on a 4-byte boundary */
  assert((uintptr_t)buffer % 4 == 0);
  /* Buffer size must be aligned on a 4-byte boundary */
  assert(length % 4 == 0);

  struct Sdio * const interface = object;
  STM_SDIO_Type * const reg = interface->base.reg;

  interface->count = length;
  dmaAppend(interface->txDma, (void *)&reg->FIFO, buffer, length >> 2);

  execute(interface, interface->command, interface->argument);
  return length;
}
