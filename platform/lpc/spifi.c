/*
 * spifi.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/gpdma_oneshot.h>
#include <halm/platform/lpc/spifi.h>
#include <halm/platform/lpc/spifi_defs.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
enum
{
  STATUS_OK,
  STATUS_RX_BUSY,
  STATUS_TX_BUSY,
  STATUS_ERROR
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Spifi *, uint8_t);
static void enableIndirectMode(struct Spifi *);
static void enableMemoryMappingMode(struct Spifi *);
static void executeCommand(struct Spifi *, bool);
static uint32_t makeCommand(const struct Spifi *);
static void resetContext(struct Spifi *);
static void resetMode(struct Spifi *);
static void dmaInterruptHandler(void *);
static void spifiInterruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result spifiInit(void *, const void *);
static void spifiDeinit(void *);
static void spifiSetCallback(void *, void (*)(void *), void *);
static enum Result spifiGetParam(void *, int, void *);
static enum Result spifiSetParam(void *, int, const void *);
static size_t spifiRead(void *, void *, size_t);
static size_t spifiWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Spifi = &(const struct InterfaceClass){
    .size = sizeof(struct Spifi),
    .init = spifiInit,
    .deinit = spifiDeinit,

    .setCallback = spifiSetCallback,
    .getParam = spifiGetParam,
    .setParam = spifiSetParam,
    .read = spifiRead,
    .write = spifiWrite
};
/*----------------------------------------------------------------------------*/
static bool dmaSetup(struct Spifi *interface, uint8_t channel)
{
  const struct GpDmaOneShotConfig dmaConfigs[] = {
      {
          .event = GPDMA_SPIFI,
          .type = GPDMA_TYPE_P2M,
          .channel = channel
      }, {
          .event = GPDMA_SPIFI,
          .type = GPDMA_TYPE_M2P,
          .channel = channel
      }
  };
  static const struct GpDmaSettings dmaSettings[] = {
      {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_WORD,
              .increment = false
          },
          .destination = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_WORD,
              .increment = true
          }
      }, {
          .source = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_WORD,
              .increment = true
          },
          .destination = {
              .burst = DMA_BURST_1,
              .width = DMA_WIDTH_WORD,
              .increment = false
          }
      }
  };

  interface->rxDma = init(GpDmaOneShot, &dmaConfigs[0]);
  if (!interface->rxDma)
    return false;

  interface->txDma = init(GpDmaOneShot, &dmaConfigs[1]);
  if (!interface->txDma)
    return false;

  dmaConfigure(interface->rxDma, &dmaSettings[0]);
  dmaSetCallback(interface->rxDma, dmaInterruptHandler, interface);

  dmaConfigure(interface->txDma, &dmaSettings[1]);
  dmaSetCallback(interface->txDma, dmaInterruptHandler, interface);

  return true;
}
/*----------------------------------------------------------------------------*/
static void enableIndirectMode(struct Spifi *interface)
{
  LPC_SPIFI_Type * const reg = interface->base.reg;

  if (interface->blocking)
    reg->CTRL &= ~CTRL_DMAEN;
  else
    reg->CTRL |= CTRL_DMAEN;

  interface->memmap = false;
}
/*----------------------------------------------------------------------------*/
static void enableMemoryMappingMode(struct Spifi *interface)
{
  LPC_SPIFI_Type * const reg = interface->base.reg;
  uint32_t command = makeCommand(interface);

  /* Ensure that poll mode was disabled */
  assert(!(command & CMD_POLL));
  /* Clear data length field */
  command &= ~CMD_DATALEN_MASK;

  /* Disable DMA mode */
  reg->CTRL &= ~CTRL_DMAEN;
  /* Enable memory-mapped mode */
  reg->MCMD = command;
  /* Wait until the peripheral enters memory-mapped mode */
  while (!(reg->STAT & STAT_MCINIT));

  interface->memmap = true;
}
/*----------------------------------------------------------------------------*/
static void executeCommand(struct Spifi *interface, bool out)
{
  LPC_SPIFI_Type * const reg = interface->base.reg;
  uint32_t command = makeCommand(interface);

  if (out)
  {
    /* Poll mode must not be used with data write commands */
    assert(!(command & CMD_POLL));

    command |= CMD_DOUT;
  }

  if (interface->address.length)
    reg->ADDR = interface->address.value;
  if (interface->post.length)
    reg->IDATA = interface->post.value;
  reg->CMD = command;
}
/*----------------------------------------------------------------------------*/
static uint32_t makeCommand(const struct Spifi *interface)
{
  enum FieldPacking
  {
    PACK_PARALLEL,
    PACK_SERIAL,
    PACK_DISABLED
  };

  static const uint8_t FRAMEFORM_NO_OPCODE[] = {
      FRAMEFORM_OPCODE,
      FRAMEFORM_OPCODE_ADDRESS_8,
      FRAMEFORM_OPCODE_ADDRESS_16,
      FRAMEFORM_OPCODE_ADDRESS_24,
      FRAMEFORM_OPCODE_ADDRESS_32
  };

  const uint8_t delay = interface->delay.length + interface->post.length;
  enum FieldPacking packCommand = PACK_DISABLED;
  enum FieldPacking packAddress = PACK_DISABLED;
  enum FieldPacking packPost = PACK_DISABLED;
  enum FieldPacking packData = PACK_DISABLED;
  uint8_t fieldform;
  uint8_t frameform;

  if (delay != 0)
  {
    assert(delay <= CMD_INTLEN_MAX);

    if (interface->post.length != 0 && interface->delay.length != 0)
    {
      assert(interface->post.serial == interface->delay.serial);
      packPost = interface->post.serial ? PACK_SERIAL : PACK_PARALLEL;
    }
    else
    {
      const bool serial = interface->post.length != 0 ?
          interface->post.serial : interface->delay.serial;

      packPost = serial ? PACK_SERIAL : PACK_PARALLEL;
    }
  }

  if (interface->command.length != 0)
  {
    frameform = FRAMEFORM_NO_OPCODE[interface->address.length];
    packCommand = interface->command.serial ? PACK_SERIAL : PACK_PARALLEL;
  }
  else
  {
    assert(interface->address.length >= 3);

    frameform = interface->address.length == 4 ?
        FRAMEFORM_ADDRESS_32 : FRAMEFORM_ADDRESS_24;
  }

  if (interface->address.length != 0)
    packAddress = interface->address.serial ? PACK_SERIAL : PACK_PARALLEL;
  packData = interface->data.serial ? PACK_SERIAL : PACK_PARALLEL;

  assert(
      (
          /* FIELDFORM_PARALLEL_ALL or FIELDFORM_SERIAL_OPCODE */
          packAddress != PACK_SERIAL
          && packPost != PACK_SERIAL
          && packData != PACK_SERIAL
      ) || (
          /* FIELDFORM_PARALLEL_DATA or FIELDFORM_SERIAL_ALL */
          packCommand != PACK_PARALLEL
          && packAddress != PACK_PARALLEL
          && packPost != PACK_PARALLEL
      )
  );

  if (packAddress != PACK_SERIAL && packPost != PACK_SERIAL
      && packData != PACK_SERIAL)
  {
    if (packCommand != PACK_SERIAL)
      fieldform = FIELDFORM_PARALLEL_ALL;
    else
      fieldform = FIELDFORM_SERIAL_OPCODE;
  }
  else
  {
    if (packData != PACK_SERIAL)
      fieldform = FIELDFORM_PARALLEL_DATA;
    else
      fieldform = FIELDFORM_SERIAL_ALL;
  }

  uint32_t command = CMD_DATALEN(interface->data.length)
      | CMD_FIELDFORM(fieldform) | CMD_FRAMEFORM(frameform);

  if (interface->poll)
    command |= CMD_POLL;
  if (interface->command.length)
    command |= CMD_OPCODE(interface->command.value);
  if (delay)
    command |= CMD_INTLEN(delay);

  return command;
}
/*----------------------------------------------------------------------------*/
static void resetContext(struct Spifi *interface)
{
  interface->command.length = 0;
  interface->command.serial = true;
  interface->command.value = 0;

  interface->address.length = 0;
  interface->address.serial = true;
  interface->address.value = 0;

  interface->post.length = 0;
  interface->post.serial = true;
  interface->post.value = 0;

  interface->delay.length = 0;
  interface->delay.serial = true;

  interface->data.length = 0;
  interface->data.response = 0;
  interface->data.serial = true;
}
/*----------------------------------------------------------------------------*/
static void resetMode(struct Spifi *interface)
{
  LPC_SPIFI_Type * const reg = interface->base.reg;

  if (reg->STAT & (STAT_MCINIT | STAT_CMD))
  {
    reg->STAT = STAT_RESET;
    while (reg->STAT & (STAT_MCINIT | STAT_CMD));
  }
}
/*----------------------------------------------------------------------------*/
static void dmaInterruptHandler(void *object)
{
  struct Spifi * const interface = object;
  enum Result res = E_INVALID;

  switch (interface->status)
  {
    case STATUS_RX_BUSY:
      res = dmaStatus(interface->rxDma);
      break;

    case STATUS_TX_BUSY:
      res = dmaStatus(interface->txDma);
      break;

    default:
      break;
  }

  assert(res == E_OK || res == E_ERROR);

  if (interface->sync)
  {
    if (interface->status != STATUS_ERROR)
      interface->status = res == E_OK ? STATUS_OK : STATUS_ERROR;

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
  else
  {
    if (res != E_OK)
      interface->status = STATUS_ERROR;
    interface->sync = true;
  }
}
/*----------------------------------------------------------------------------*/
static void spifiInterruptHandler(void *object)
{
  /*
   * Handling of non-blocking operations:
   *   - write, no data: IRQ
   *   - write, data:    IRQ + DMA
   *   - read, no data:  IRQ
   *   - read, data:     IRQ + DMA
   */

  struct Spifi * const interface = object;
  LPC_SPIFI_Type * const reg = interface->base.reg;

  /* Clear pending interrupt flag */
  reg->STAT = STAT_INTRQ;

  if (interface->sync)
  {
    if (interface->status != STATUS_ERROR)
      interface->status = STATUS_OK;

    if (interface->poll)
    {
      interface->data.response = (uint32_t)reg->DATA_B;
      interface->poll = false;
    }

    if (interface->callback)
      interface->callback(interface->callbackArgument);
  }
  else
  {
    interface->sync = true;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spifiInit(void *object, const void *configBase)
{
  const struct SpifiConfig * const config = configBase;
  assert(config);
  assert(config->delay <= CTRL_CSHIGH_MAX + 1);
  assert(config->timeout <= CTRL_TIMEOUT_MAX + 1);
  assert(config->mode == 0 || config->mode == 3);

  const struct SpifiBaseConfig baseConfig = {
      .cs = config->cs,
      .io0 = config->io0,
      .io1 = config->io1,
      .io2 = config->io2,
      .io3 = config->io3,
      .sck = config->sck
  };
  struct Spifi * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SpifiBase->init(interface, &baseConfig)) != E_OK)
    return res;

  if (!dmaSetup(interface, config->dma))
    return E_ERROR;

  interface->base.handler = spifiInterruptHandler;
  interface->callback = 0;
  interface->status = STATUS_OK;
  interface->blocking = true;
  interface->memmap = true;
  interface->poll = false;
  resetContext(interface);

  LPC_SPIFI_Type * const reg = interface->base.reg;
  uint32_t control = 0;

  if (config->delay)
    control |= CTRL_CSHIGH(config->delay - 1);
  else
    control |= CTRL_CSHIGH(CTRL_CSHIGH_MAX);

  if (config->timeout)
    control |= CTRL_TIMEOUT(config->timeout - 1);
  else
    control |= CTRL_TIMEOUT(CTRL_TIMEOUT_MAX);

  // TODO Configure RFCLK, FBCLK
  if (config->mode == 3)
    control |= CTRL_MODE3;
  else
    control |= CTRL_FBCLK;

  if (!interface->base.wide)
    control |= CTRL_DUAL;

  reg->CTRL = control;
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spifiDeinit(void *object)
{
  struct Spifi * const interface = object;

  /* Disable memory-mapped mode or stop current command */
  resetMode(interface);

  /* Disable SPIFI IRQ */
  irqDisable(interface->base.irq);

  deinit(interface->txDma);
  deinit(interface->rxDma);
  SpifiBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static void spifiSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Spifi * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result spifiGetParam(void *object, int parameter, void *data)
{
  struct Spifi * const interface = object;
  LPC_SPIFI_Type * const reg = interface->base.reg;

  switch ((enum SPIMParameter)parameter)
  {
    case IF_SPIM_RESPONSE:
      *(uint32_t *)data = (uint32_t)interface->data.response;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_STATUS:
      if (!interface->memmap)
      {
        if (!interface->blocking)
        {
          switch (interface->status)
          {
            case STATUS_OK:
              return E_OK;

            case STATUS_ERROR:
              return E_INTERFACE;

            default:
              return E_BUSY;
          }
        }
        else
          return reg->STAT & STAT_CMD ? E_BUSY : E_OK;
      }
      else
        return E_BUSY;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spifiSetParam(void *object, int parameter, const void *data)
{
  struct Spifi * const interface = object;
  LPC_SPIFI_Type * const reg = interface->base.reg;

  switch ((enum SPIMParameter)parameter)
  {
    case IF_SPIM_MODE:
    {
      const uint8_t mode = *(const uint8_t *)data;

      if (mode != 0 && mode != 3)
        return E_VALUE;

      if (mode == 0)
        reg->CTRL &= ~CTRL_MODE3;
      else
        reg->CTRL |= CTRL_MODE3;

      return E_OK;
    }

    case IF_SPIM_DUAL:
      reg->CTRL |= CTRL_DUAL;
      return E_OK;

    case IF_SPIM_QUAD:
      if (interface->base.wide)
      {
        reg->CTRL &= ~CTRL_DUAL;
        return E_OK;
      }
      else
        return E_VALUE;

    case IF_SPIM_INDIRECT:
      resetMode(interface);
      enableIndirectMode(interface);
      return E_OK;

    case IF_SPIM_MEMORY_MAPPED:
      resetMode(interface);
      enableMemoryMappingMode(interface);
      return E_OK;

    case IF_SPIM_COMMAND:
      interface->command.length = 1;
      interface->command.value = *(const uint8_t *)data;
      return E_OK;

    case IF_SPIM_COMMAND_NONE:
      interface->command.length = 0;
      return E_OK;

    case IF_SPIM_COMMAND_PARALLEL:
      interface->command.serial = false;
      return E_OK;

    case IF_SPIM_COMMAND_SERIAL:
      interface->command.serial = true;
      return E_OK;

    case IF_SPIM_DELAY_LENGTH:
    {
      const uint8_t value = *(const uint8_t *)data;

      if (value <= CMD_INTLEN_MAX)
      {
        interface->delay.length = value;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    case IF_SPIM_DELAY_NONE:
      interface->delay.length = 0;
      return E_OK;

    case IF_SPIM_DELAY_PARALLEL:
      interface->delay.serial = false;
      return E_OK;

    case IF_SPIM_DELAY_SERIAL:
      interface->delay.serial = true;
      return E_OK;

    case IF_SPIM_ADDRESS_24:
      interface->address.length = 3;
      interface->address.value = *(const uint32_t *)data;
      return E_OK;

    case IF_SPIM_ADDRESS_32:
      interface->address.length = 4;
      interface->address.value = *(const uint32_t *)data;
      return E_OK;

    case IF_SPIM_ADDRESS_NONE:
      interface->address.length = 0;
      return E_OK;

    case IF_SPIM_ADDRESS_PARALLEL:
      interface->address.serial = false;
      return E_OK;

    case IF_SPIM_ADDRESS_SERIAL:
      interface->address.serial = true;
      return E_OK;

    case IF_SPIM_POST_ADDRESS_8:
      interface->post.length = 1;
      interface->post.value = *(const uint32_t *)data;
      return E_OK;

    case IF_SPIM_POST_ADDRESS_NONE:
      interface->post.length = 0;
      return E_OK;

    case IF_SPIM_POST_ADDRESS_PARALLEL:
      interface->post.serial = false;
      return E_OK;

    case IF_SPIM_POST_ADDRESS_SERIAL:
      interface->post.serial = true;
      return E_OK;

    case IF_SPIM_DATA_LENGTH:
    {
      const uint32_t value = *(const uint32_t *)data;

      if (value <= CMD_DATALEN_MAX)
      {
        interface->data.length = value;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    case IF_SPIM_DATA_NONE:
      interface->data.length = 0;
      return E_OK;

    case IF_SPIM_DATA_POLL_BIT:
    {
      const uint8_t value = *(const uint8_t *)data;

      if (value < 8)
      {
        interface->data.length = value;
        interface->poll = true;
        return E_OK;
      }
      else
        return E_VALUE;
    }

    case IF_SPIM_DATA_PARALLEL:
      interface->data.serial = false;
      return E_OK;

    case IF_SPIM_DATA_SERIAL:
      interface->data.serial = true;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_BLOCKING:
      reg->CTRL &= ~(CTRL_DMAEN | CTRL_INTEN);
      interface->blocking = true;
      return E_OK;

    case IF_ZEROCOPY:
      reg->STAT = STAT_INTRQ;
      reg->CTRL |= CTRL_DMAEN | CTRL_INTEN;
      interface->blocking = false;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t spifiRead(void *object, void *buffer, size_t length)
{
  struct Spifi * const interface = object;
  LPC_SPIFI_Type * const reg = interface->base.reg;

  if (!interface->blocking)
  {
    assert(length % 4 == 0);
    interface->sync = length == 0;
  }

  executeCommand(interface, false);

  if (length)
  {
    /* Poll mode requires zero length read */
    assert(!interface->poll);

    if (interface->blocking)
    {
      uint8_t *position = buffer;
      size_t left = length;

      while (left--)
        *position++ = reg->DATA_B;
    }
    else
    {
      dmaAppend(interface->rxDma, buffer, (const void *)&reg->DATA, length / 4);

      interface->status = STATUS_RX_BUSY;
      if (dmaEnable(interface->rxDma) != E_OK)
      {
        interface->status = STATUS_ERROR;
        return 0;
      }
    }
  }

  if (interface->blocking)
  {
    while (reg->STAT & STAT_CMD);
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t spifiWrite(void *object, const void *buffer, size_t length)
{
  struct Spifi * const interface = object;
  LPC_SPIFI_Type * const reg = interface->base.reg;

  if (!interface->blocking)
  {
    assert(length % 4 == 0);
    interface->sync = length == 0;
  }

  executeCommand(interface, true);

  if (length)
  {
    if (interface->blocking)
    {
      const uint8_t *position = buffer;
      size_t left = length;

      while (left--)
        reg->DATA_B = *position++;
    }
    else
    {
      dmaAppend(interface->txDma, (void *)&reg->DATA, buffer, length / 4);

      interface->status = STATUS_TX_BUSY;
      if (dmaEnable(interface->txDma) != E_OK)
      {
        interface->status = STATUS_ERROR;
        return 0;
      }
    }
  }

  if (interface->blocking)
  {
    while (reg->STAT & STAT_CMD);
  }

  return length;
}
