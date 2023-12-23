/*
 * spim.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/spim.h>
#include <halm/platform/numicro/spim_defs.h>
#include <halm/platform/platform_defs.h>
#include <halm/timer.h>
#include <xcore/memory.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_POLL_RATE 100

enum
{
  STATUS_OK,
  STATUS_RX_BUSY,
  STATUS_TX_BUSY,
  STATUS_ERROR
};
/*----------------------------------------------------------------------------*/
static void enableMemoryMappingMode(struct Spim *);
static void enableNormalMode(struct Spim *);
static void executeDirectCommand(struct Spim *, uintptr_t, bool);
static void executeDmaCommand(struct Spim *, uintptr_t, bool);
static void readDataDirect(struct Spim *, void *, size_t);
static bool readPollResponse(struct Spim *);
static void resetContext(struct Spim *);
static void resetMode(struct Spim *);
static uint32_t getSerialRate(const struct Spim *);
static void setSerialRate(struct Spim *, uint32_t);
static void spimInterruptHandler(void *);
static void timerInterruptHandler(void *);
static void writeDataDirect(struct Spim *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static enum Result spimInit(void *, const void *);
static void spimDeinit(void *);
static void spimSetCallback(void *, void (*)(void *), void *);
static enum Result spimGetParam(void *, int, void *);
static enum Result spimSetParam(void *, int, const void *);
static size_t spimRead(void *, void *, size_t);
static size_t spimWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Spim = &(const struct InterfaceClass){
    .size = sizeof(struct Spim),
    .init = spimInit,
    .deinit = spimDeinit,

    .setCallback = spimSetCallback,
    .getParam = spimGetParam,
    .setParam = spimSetParam,
    .read = spimRead,
    .write = spimWrite
};
/*----------------------------------------------------------------------------*/
static void enableMemoryMappingMode(struct Spim *interface)
{
  NM_SPIM_Type * const reg = interface->base.reg;
  uint32_t ctl0 = reg->CTL0;
  uint32_t ctl2 = reg->CTL2;
  uint32_t dmmctl = reg->DMMCTL;

  /* Clock divider must not be set to zero for DTR/DDR mode */
  assert(!interface->ddr || CTL1_DIVIDER_VALUE(reg->CTL1) > 0);

  /*
   * Accepted command codes:
   *   03h - Standard Read
   *   0Bh - Fast Read
   *   3Bh - Fast Read Dual Output
   *   BBh - Fast Read Dual I/O
   *   EBh - Fast Read Quad I/O
   *   E7h - Word Read Quad I/O
   *   0Dh - DTR/DDR Fast Read
   *   BDh - DTR/DDR Fast Read Dual I/O
   *   EDh - DTR/DDR Fast Read Quad I/O
   */
  if (interface->ddr)
  {
    const uint8_t cmd = interface->command.value;
    (void)cmd;

    assert((interface->data.serial && cmd == 0x0D)
        || (!interface->data.serial && interface->quad && cmd == 0xED)
        || (!interface->data.serial && !interface->quad && cmd == 0xBD));
  }
  else
  {
    const uint8_t cmd = interface->command.value;
    (void)cmd;

    assert((interface->data.serial
    				&& (cmd == 0x03 || cmd == 0x0B))
        || (!interface->data.serial && interface->quad
            && (cmd == 0xE7 || cmd == 0xEB))
        || (!interface->data.serial && !interface->quad
            && (cmd == 0x3B || cmd == 0xBB)));
  }

  /* CTL0 */
  ctl0 &= ~(CTL0_OPMODE_MASK | CTL0_CMDCODE_MASK | CTL0_B4ADDREN);
  if (interface->address.length == 4)
    ctl0 |= CTL0_B4ADDREN;
  ctl0 |= CTL0_CMDCODE(interface->command.value) | CTL0_OPMODE(OPMODE_DMM);

  /* CTL2 and DMMCTL */
  ctl2 &= ~(CTL2_DTRMPOFF | CTL2_DCNUM_MASK);
  dmmctl &= ~(DMMCTL_CRMDAT_MASK | DMMCTL_CREN);

  uint32_t cycles = 4;

  if (interface->ddr)
    cycles >>= 1;
  if (interface->quad)
    cycles >>= 1;

  if (interface->post.length)
  {
    /* Command 03h does not support mode phase */
    assert(interface->command.value != 0x03);

    if (interface->command.value == 0xBB)
      cycles *= interface->delay.length + interface->post.length;
    else
      cycles *= interface->delay.length;

    dmmctl |= DMMCTL_CRMDAT(interface->post.value);
  }
  else
  {
    if ((interface->command.value & 0x0F) == 0x0D)
    {
      /* Disable mode phase for DTR/DDR commands only */
      ctl2 |= CTL2_DTRMPOFF;
    }

    cycles *= interface->delay.length;
  }
  ctl2 |= CTL2_DCNUM(cycles);

  // TODO
  // if (interface->command.value == 0xEB || interface->command.value == 0xE7)
  //   dmmctl |= DMMCTL_BWEN;

  /* Clear cache */
  reg->CTL1 |= CTL1_CDINVAL;
  while (reg->CTL1 & CTL1_CDINVAL);

  /* Configure memory mapping mode */
  reg->DMMCTL = dmmctl;
  reg->CTL2 = ctl2;

  /* Start Direct Memory Mapping Mode */
  reg->CTL0 = ctl0;

  interface->memmap = true;
}
/*----------------------------------------------------------------------------*/
static void enableNormalMode(struct Spim *interface)
{
  NM_SPIM_Type * const reg = interface->base.reg;

  /* Enter Normal I/O mode */
  reg->CTL0 &= ~CTL0_OPMODE_MASK;

  interface->memmap = false;
}
/*----------------------------------------------------------------------------*/
static void executeDirectCommand(struct Spim *interface, uintptr_t buffer,
    bool out)
{
  NM_SPIM_Type * const reg = interface->base.reg;
  uint32_t ctl0 = reg->CTL0;

  ctl0 &= ~(CTL0_QDIODIR | CTL0_BITMODE_MASK | CTL0_OPMODE_MASK);
  ctl0 |= CTL0_OPMODE(OPMODE_NORMAL);

  /* Enter Normal I/O mode and activate Slave Select */
  reg->DMMCTL &= ~(DMMCTL_CRMDAT_MASK | DMMCTL_CREN);
  reg->CTL0 = ctl0;
  reg->CTL1 &= ~CTL1_SS;

  if (interface->command.length)
  {
    uint32_t bitmode;

    /* Send command part */
    if (interface->command.serial)
      bitmode = CTL0_BITMODE(BITMODE_STANDARD);
    else if (!interface->quad)
      bitmode = CTL0_BITMODE(BITMODE_DUAL);
    else
      bitmode = CTL0_BITMODE(BITMODE_QUAD);

    reg->CTL0 = ctl0 | bitmode | CTL0_QDIODIR;
    writeDataDirect(interface, &interface->command.value, 1);
  }

  if (interface->address.length)
  {
    uint32_t address = toBigEndian32(interface->address.value);
    uint32_t bitmode;

    address >>= 32 - interface->address.length * 8;

    /* Send address part */
    if (interface->address.serial)
      bitmode = CTL0_BITMODE(BITMODE_STANDARD);
    else if (!interface->quad)
      bitmode = CTL0_BITMODE(BITMODE_DUAL);
    else
      bitmode = CTL0_BITMODE(BITMODE_QUAD);

    reg->CTL0 = ctl0 | bitmode | CTL0_QDIODIR;
    writeDataDirect(interface, &address, interface->address.length);
  }

  if (interface->post.length)
  {
    uint32_t bitmode;

    /* Send post-address part */
    if (interface->post.serial)
      bitmode = CTL0_BITMODE(BITMODE_STANDARD);
    else if (!interface->quad)
      bitmode = CTL0_BITMODE(BITMODE_DUAL);
    else
      bitmode = CTL0_BITMODE(BITMODE_QUAD);

    reg->CTL0 = ctl0 | bitmode | CTL0_QDIODIR;
    writeDataDirect(interface, &interface->post.value, 1);
  }

  if (interface->delay.length)
  {
    uint32_t bitmode;
    uint8_t pattern[CTL2_DCNUM_MAX + 1];

    /* Send dummy bytes */
    if (interface->delay.serial)
      bitmode = CTL0_BITMODE(BITMODE_STANDARD);
    else if (!interface->quad)
      bitmode = CTL0_BITMODE(BITMODE_DUAL);
    else
      bitmode = CTL0_BITMODE(BITMODE_QUAD);
    memset(pattern, 0xFF, sizeof(pattern));

    reg->CTL0 = ctl0 | bitmode | CTL0_QDIODIR;
    writeDataDirect(interface, pattern, interface->delay.length);
  }

  if (interface->data.length || interface->poll)
  {
    /* Send or receive data bytes */
    if (interface->data.serial)
      ctl0 |= CTL0_BITMODE(BITMODE_STANDARD);
    else if (!interface->quad)
      ctl0 |= CTL0_BITMODE(BITMODE_DUAL);
    else
      ctl0 |= CTL0_BITMODE(BITMODE_QUAD);

    if (out)
      ctl0 |= CTL0_QDIODIR;

    reg->CTL0 = ctl0;

    if (interface->data.length)
    {
      if (out)
      {
        writeDataDirect(interface, (const void *)buffer,
            interface->data.length);
      }
      else
      {
        readDataDirect(interface, (void *)buffer,
            interface->data.length);
      }
    }
  }

  const bool invoke = !interface->blocking && interface->callback != NULL;

  if (!interface->poll)
  {
    /* Deactivate Slave Select in all modes except for polling mode */
    reg->CTL1 |= CTL1_SS;
  }
  else if (!invoke)
  {
    /* Read first poll response immediately in case of blocking mode */
    readPollResponse(interface);
  }

  if (invoke)
  {
    /* Call user callback from interrupt handler */
    irqSetPending(interface->base.irq);
  }
}
/*----------------------------------------------------------------------------*/
static void executeDmaCommand(struct Spim *interface, uintptr_t buffer,
    bool out)
{
  NM_SPIM_Type * const reg = interface->base.reg;
  uint32_t ctl0 = reg->CTL0;
  uint32_t ctl2 = reg->CTL2;

  /* Clock divider must not be set to zero for DTR/DDR mode */
  assert(!interface->ddr || CTL1_DIVIDER_VALUE(reg->CTL1) > 0);

  /*
   * Accepted command codes:
   *   03h - Standard Read
   *   0Bh - Fast Read
   *   3Bh - Fast Read Dual Output
   *   BBh - Fast Read Dual I/O
   *   EBh - Fast Read Quad I/O
   *   E7h - Word Read Quad I/O
   *   0Dh - DTR/DDR Fast Read
   *   BDh - DTR/DDR Fast Read Dual I/O
   *   EDh - DTR/DDR Fast Read Quad I/O
   *
   *   02h - Page Program
   *   32h - Page Program Quad Output
   *   38h - Page Program Quad I/O
   *   40h - Page Program QPI
   */
  if (interface->ddr)
  {
    const uint8_t cmd = interface->command.value;
    (void)cmd;

    assert((interface->data.serial && cmd == 0x0D)
        || (!interface->data.serial && interface->quad && cmd == 0xED)
        || (!interface->data.serial && !interface->quad && cmd == 0xBD));
  }
  else
  {
    const uint8_t cmd = interface->command.value;
    (void)cmd;

    /* Check read commands */
    assert(out
        || (interface->data.serial
    				&& (cmd == 0x03 || cmd == 0x0B))
        || (!interface->data.serial && interface->quad
            && (cmd == 0xE7 || cmd == 0xEB))
        || (!interface->data.serial && !interface->quad
            && (cmd == 0x3B || cmd == 0xBB)));

    /* Check write commands */
    assert(!out
        || (interface->data.serial
            && (cmd == 0x02))
        || (!interface->data.serial && interface->quad
            && (cmd == 0x32 || cmd == 0x38 || cmd == 0x40)));
  }

  /* CTL0 */
  ctl0 &= ~(CTL0_B4ADDREN | CTL0_OPMODE_MASK | CTL0_CMDCODE_MASK);
  ctl0 |= CTL0_CMDCODE(interface->command.value);
  ctl0 |= out ? CTL0_OPMODE(OPMODE_DMA_WRITE) : CTL0_OPMODE(OPMODE_DMA_READ);
  if (interface->address.length == 4)
    ctl0 |= CTL0_B4ADDREN;

  /* CTL2 */
  ctl2 &= ~(CTL2_DTRMPOFF | CTL2_DCNUM_MASK);

  if (!out)
  {
    uint32_t cycles = 4;

    /* Continuous Read mode is not supported in non-memory-mapped mode */
    assert((interface->post.length == 1 && interface->post.value == 0xFF)
        || interface->post.length == 0);

    if (interface->ddr)
      cycles >>= 1;
    if (interface->quad)
      cycles >>= 1;
    cycles *= interface->delay.length;

    if (interface->ddr)
      ctl2 |= CTL2_DTRMPOFF;
    ctl2 |= CTL2_DCNUM(cycles);
  }

  /* Prepare DMA mode and activate Slave Select */
  reg->FADDR = interface->address.value;
  reg->SRAMADDR = (uint32_t)buffer;
  reg->DMACNT = interface->data.length;
  reg->DMMCTL &= ~(DMMCTL_CRMDAT_MASK | DMMCTL_CREN);
  reg->CTL0 = ctl0;
  reg->CTL2 = ctl2;

  if (!interface->blocking)
  {
    /* Enable interrupt in non-blocking mode */
    reg->CTL0 |= CTL0_IEN | CTL0_IF;
  }

  /* Start command */
  reg->CTL1 &= ~CTL1_SS;
  reg->CTL1 |= CTL1_SPIMEN;

  if (interface->blocking)
  {
    /* Wait until transfer is completed */
    while (reg->CTL1 & CTL1_SPIMEN);
    /* Deactivate Slave Select */
    reg->CTL1 |= CTL1_SS;
  }
}
/*----------------------------------------------------------------------------*/
static void readDataDirect(struct Spim *interface, void *buffer, size_t length)
{
  NM_SPIM_Type * const reg = interface->base.reg;
  uint8_t *position = buffer;

  while (length)
  {
    size_t words = MIN(length, 16) >> 2;
    uint32_t ctl0 = reg->CTL0 & ~(CTL0_BURSTNUM_MASK | CTL0_DWIDTH_MASK);

    if (words > 1)
    {
      ctl0 |= CTL0_BURSTNUM(words - 1) | CTL0_DWIDTH(DWIDTH_32);
    }
    else
    {
      ctl0 |= CTL0_BURSTNUM(length - 1) | CTL0_DWIDTH(DWIDTH_8);
    }

    reg->CTL0 = ctl0;
    reg->CTL1 |= CTL1_SPIMEN;
    while (reg->CTL1 & CTL1_SPIMEN);

    if (words > 1)
    {
      length -= words << 2;

      while (words--)
      {
        const uint32_t tmp = reg->RX[words];
        memcpy(position, &tmp, sizeof(tmp));

        position += sizeof(tmp);
      }
    }
    else
    {
      /* Data is transmitted LSB first when BURSTNUM = 0 */
      while (length--)
        *position++ = reg->RX[length];
      length = 0;
    }
  }

  /* Clear interrupt flag */
  reg->CTL0 |= CTL0_IF;
}
/*----------------------------------------------------------------------------*/
static bool readPollResponse(struct Spim *interface)
{
  uint8_t response;

  readDataDirect(interface, &response, sizeof(response));

  if ((response & BIT(interface->data.length)) == 0)
  {
    NM_SPIM_Type * const reg = interface->base.reg;

    /* Deactivate Slave Select */
    reg->CTL1 |= CTL1_SS;

    interface->data.response = response;
    interface->poll = false;

    return true;
  }
  else
  {
    timerSetValue(interface->timer, 0);
    timerEnable(interface->timer);

    return false;
  }
}
/*----------------------------------------------------------------------------*/
static void resetContext(struct Spim *interface)
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
static void resetMode(struct Spim *interface)
{
  NM_SPIM_Type * const reg = interface->base.reg;

  /* Wait until current operation is finished */
  while (reg->CTL1 & CTL1_SPIMEN);

  reg->CTL0 = (reg->CTL0 & ~CTL0_OPMODE_MASK) | CTL0_OPMODE(OPMODE_NORMAL);
  reg->CTL1 |= CTL1_SS;
}
/*----------------------------------------------------------------------------*/
static uint32_t getSerialRate(const struct Spim *interface)
{
  const NM_SPIM_Type * const reg = interface->base.reg;
  const uint32_t clock = spimGetClock(&interface->base);
  const uint32_t divider = CTL1_DIVIDER_VALUE(reg->CTL1);

  return divider ? clock / (divider << 1) : clock;
}
/*----------------------------------------------------------------------------*/
static void setSerialRate(struct Spim *interface, uint32_t rate)
{
  NM_SPIM_Type * const reg = interface->base.reg;
  const uint32_t clock = spimGetClock(&interface->base);
  uint32_t divider = 0;

  if (rate != 0)
  {
    divider = ((clock >> 1) + (rate - 1)) / rate;

    if (divider & 1)
      ++divider;
    if (divider > CTL1_DIVIDER_MAX)
      divider = CTL1_DIVIDER_MAX;
  }

  reg->CTL1 = (reg->CTL1 & ~CTL1_DIVIDER_MASK) | CTL1_DIVIDER(divider);
}
/*----------------------------------------------------------------------------*/
static void spimInterruptHandler(void *object)
{
  struct Spim * const interface = object;
  NM_SPIM_Type * const reg = interface->base.reg;
  bool event;

  if ((reg->CTL0 & (CTL0_IEN | CTL0_IF)) == (CTL0_IEN | CTL0_IF))
  {
    /* Disable interrupt, clear pending interrupt flag */
    reg->CTL0 = (reg->CTL0 & ~CTL0_IEN) | CTL0_IF;
    /* Deactivate Slave Select */
    reg->CTL1 |= CTL1_SS;
  }

  if (interface->poll)
    event = readPollResponse(interface);
  else
	  event = true;

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void timerInterruptHandler(void *object)
{
  struct Spim * const interface = object;
  const bool event = readPollResponse(interface);

  if (event && interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void writeDataDirect(struct Spim *interface, const void *buffer,
    size_t length)
{
  NM_SPIM_Type * const reg = interface->base.reg;
  const uint8_t *position = buffer;

  while (length)
  {
    size_t words = MIN(length, 16) >> 2;
    uint32_t ctl0 = reg->CTL0 & ~(CTL0_BURSTNUM_MASK | CTL0_DWIDTH_MASK);

    if (words > 1)
    {
      /* Data is transmitted MSB first when BURSTNUM > 0 */
      ctl0 |= CTL0_BURSTNUM(words - 1) | CTL0_DWIDTH(DWIDTH_32);
      length -= words << 2;

      while (words--)
      {
        uint32_t tmp;

        memcpy(&tmp, position, sizeof(tmp));
        reg->TX[words] = tmp;

        position += sizeof(tmp);
      }
    }
    else
    {
      /* Data is transmitted LSB first when BURSTNUM = 0 */
      ctl0 |= CTL0_BURSTNUM(length - 1) | CTL0_DWIDTH(DWIDTH_8);

      while (length--)
        reg->TX[length] = *position++;
      length = 0;
    }

    reg->CTL0 = ctl0;
    reg->CTL1 |= CTL1_SPIMEN;
    while (reg->CTL1 & CTL1_SPIMEN);
  }

  /* Clear interrupt flag */
  reg->CTL0 |= CTL0_IF;
}
/*----------------------------------------------------------------------------*/
static enum Result spimInit(void *object, const void *configBase)
{
  const struct SpimConfig * const config = configBase;
  assert(config != NULL);
  assert(config->timer != NULL);
  assert(config->delay <= CTL1_IDLETIME_MAX + 1);
  // TODO Additional requirements for cache mode and cipher mode
  assert(config->timeout <= DMMCTL_DESELTIM_MAX + 1);

  const struct SpimBaseConfig baseConfig = {
      .cs = config->cs,
      .io0 = config->io0,
      .io1 = config->io1,
      .io2 = config->io2,
      .io3 = config->io3,
      .sck = config->sck
  };
  struct Spim * const interface = object;
  enum Result res;

  /* Call base class constructor */
  if ((res = SpimBase->init(interface, &baseConfig)) != E_OK)
    return res;

  interface->base.handler = spimInterruptHandler;
  interface->callback = NULL;
  interface->timer = config->timer;
  interface->status = STATUS_OK;
  interface->blocking = true;
  interface->ddr = false;
  interface->memmap = false;
  interface->poll = false;
  interface->quad = false;
  interface->uncached = config->uncached;
  resetContext(interface);

  const uint32_t delay = config->delay ?
      config->delay : CTL1_IDLETIME_MAX;
  const uint32_t timeout = config->timeout ?
      config->timeout : DMMCTL_DESELTIM_MAX;
  NM_SPIM_Type * const reg = interface->base.reg;

  reg->CTL0 = CTL0_CIPHOFF;
  reg->CTL1 = CTL1_IDLETIME(delay);
  reg->CTL2 = CTL2_USETEN;
  reg->DMMCTL = DMMCTL_DESELTIM(timeout);
  reg->KEY1 = 0;
  reg->KEY2 = 0;

  /* Configure Clock Divider in the CTL1 register */
  setSerialRate(interface, config->rate);

  /* Configure polling timer */
  const uint32_t overflow = timerGetFrequency(interface->timer)
      / (!config->poll ? DEFAULT_POLL_RATE : config->poll);

  timerSetAutostop(interface->timer, true);
  timerSetCallback(interface->timer, timerInterruptHandler, interface);
  timerSetOverflow(interface->timer, overflow);

  /* Enable SPIM interrupts in the NVIC */
  irqSetPriority(interface->base.irq, config->priority);
  irqEnable(interface->base.irq);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void spimDeinit(void *object)
{
  struct Spim * const interface = object;

  timerDisable(interface->timer);
  timerSetCallback(interface->timer, NULL, NULL);

  /* Disable SPIM interrupts */
  irqDisable(interface->base.irq);
  /* Disable memory-mapped mode or stop current command */
  resetMode(interface);

  SpimBase->deinit(interface);
}
/*----------------------------------------------------------------------------*/
static void spimSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Spim * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result spimGetParam(void *object, int parameter, void *data)
{
  struct Spim * const interface = object;
  NM_SPIM_Type * const reg = interface->base.reg;

  switch ((enum SPIMParameter)parameter)
  {
    case IF_SPIM_MEMORY_MAPPED_ADDRESS:
      *(uintptr_t *)data = (uintptr_t)spimGetAddress(interface);
      break;

    case IF_SPIM_RESPONSE:
      *(uint32_t *)data = (uint32_t)interface->data.response;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
      *(uint32_t *)data = getSerialRate(interface);
      return E_OK;

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
          return reg->CTL1 & CTL1_SPIMEN ? E_BUSY : E_OK;
      }
      else
        return E_BUSY;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result spimSetParam(void *object, int parameter, const void *data)
{
  struct Spim * const interface = object;

  switch ((enum SPIMParameter)parameter)
  {
    case IF_SPIM_MODE:
      return *(const uint8_t *)data == 0 ? E_OK : E_VALUE;

    case IF_SPIM_DUAL:
      interface->quad = false;
      return E_OK;

    case IF_SPIM_QUAD:
      if (interface->base.wide)
      {
        interface->quad = true;
        return E_OK;
      }
      else
        return E_VALUE;

    case IF_SPIM_SDR:
      interface->ddr = false;
      break;

    case IF_SPIM_DDR:
      interface->ddr = true;
      break;

    case IF_SPIM_INDIRECT:
      resetMode(interface);
      enableNormalMode(interface);
      return E_OK;

    case IF_SPIM_MEMORY_MAPPED:
      resetMode(interface);
      enableMemoryMappingMode(interface);
      return E_OK;

    case IF_SPIM_COMMAND:
      interface->command.value = *(const uint8_t *)data;
      interface->command.length = 1;
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

      if (value + interface->post.length <= CTL2_DCNUM_MAX)
      {
        // TODO Convert to cycles
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
      if (interface->delay.length + 1 <= CTL2_DCNUM_MAX)
      {
        interface->post.length = 1;
        interface->post.value = *(const uint32_t *)data;
        return E_OK;
      }
      else
        return E_VALUE;

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

      if (value <= SPIM_MAX_TRANSFER_SIZE)
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
    case IF_RATE:
      setSerialRate(interface, *(const uint32_t *)data);
      return E_OK;

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
static size_t spimRead(void *object, void *buffer, size_t length)
{
  struct Spim * const interface = object;

  if (interface->poll)
  {
    /* Poll mode requires zero length read */
    assert(!length && !interface->data.length);

    executeDirectCommand(interface, (uintptr_t)NULL, false);

    if (interface->blocking)
    {
      while (interface->poll)
        barrier();
    }
  }
  else
  {
    const uint8_t cmd = interface->command.value;
    bool dma = false;

    if (interface->ddr)
    {
      assert((interface->data.serial && cmd == 0x0D)
          || (!interface->data.serial && interface->quad && cmd == 0xED)
          || (!interface->data.serial && !interface->quad && cmd == 0xBD));

      dma = true;
    }
    else if (interface->data.serial)
    {
      /* Check for Standard Read and Fast Read commands */
      dma = cmd == 0x03 || cmd == 0x0B;
    }
    else if (interface->quad)
    {
      /* Check for Fast Read Quad I/O commands */
      dma = cmd == 0xEB || cmd == 0xE7;
    }
    else
    {
      /* Check for Fast Read Dual Output and Dual I/O commands */
      dma = cmd == 0x3B || cmd == 0xBB;
    }

    assert(length % 4 == 0 || !dma);
    assert(length == interface->data.length);

    if (dma)
    {
      executeDmaCommand(interface, (uintptr_t)buffer, false);
    }
    else
    {
      executeDirectCommand(interface, (uintptr_t)buffer, false);
    }
  }

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t spimWrite(void *object, const void *buffer, size_t length)
{
  struct Spim * const interface = object;
  const uint8_t cmd = interface->command.value;
  const bool dma = cmd == 0x02 || cmd == 0x32 || cmd == 0x38 || cmd == 0x40;

  assert(!interface->poll);
  assert(length % 4 == 0 || !dma);
  assert(length == interface->data.length);

  if (dma)
  {
    executeDmaCommand(interface, (uintptr_t)buffer, true);
  }
  else
  {
    executeDirectCommand(interface, (uintptr_t)buffer, true);
  }

  return length;
}
