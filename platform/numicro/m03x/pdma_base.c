/*
 * pdma_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/numicro/pdma_base.h>
#include <halm/platform/numicro/pdma_defs.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/platform_defs.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define CHANNEL_COUNT CONFIG_PLATFORM_NUMICRO_PDMA_COUNT
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const PdmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = NULL /* Default destructor */
};
/*----------------------------------------------------------------------------*/
extern unsigned long _sbss;
/*----------------------------------------------------------------------------*/
static struct PdmaBase *instances[CHANNEL_COUNT] = {NULL};
/*----------------------------------------------------------------------------*/
bool pdmaBindInstance(struct PdmaBase *channel)
{
  assert(channel != NULL);

  void *expected = NULL;

  return compareExchangePointer(&instances[channel->number],
      &expected, channel);
}
/*----------------------------------------------------------------------------*/
void pdmaUnbindInstance(struct PdmaBase *channel)
{
  assert(channel != NULL);
  instances[channel->number] = NULL;
}
/*----------------------------------------------------------------------------*/
void pdmaSetMux(struct PdmaBase *channel)
{
  volatile uint32_t * const reg = &NM_PDMA->REQSEL[channel->number >> 2];
  *reg = (*reg & channel->mux.mask) | channel->mux.value;
}
/*----------------------------------------------------------------------------*/
void pdmaStartTransfer(struct PdmaBase *channel, uint32_t control,
    uintptr_t source, uintptr_t destination, uintptr_t next)
{
  NM_PDMA_Type * const reg = channel->reg;
  NM_PDMA_CHANNEL_Type * const entry = &reg->CHANNELS[channel->number];

  entry->CTL = control;
  entry->SA = source;
  entry->DA = destination;
  entry->NEXT = DSCT_NEXT_NEXT(next);
  __dmb();

  /* Start the transfer */
  const IrqState state = irqSave();
  reg->CHCTL |= 1 << channel->number;
  irqRestore(state);
}
/*----------------------------------------------------------------------------*/
void PDMA_ISR(void)
{
  const uint32_t abtsts = NM_PDMA->ABTSTS;
  const uint32_t reqtof = INTSTS_REQTOF_VALUE(NM_PDMA->INTSTS);
  const uint32_t tdsts = NM_PDMA->TDSTS;
  const uint32_t status = abtsts | reqtof | tdsts;

  NM_PDMA->ABTSTS = abtsts;
  NM_PDMA->TDSTS = tdsts;

  for (size_t index = 0; index < ARRAY_SIZE(instances); ++index)
  {
    const uint32_t mask = 1 << index;

    if (status & mask)
    {
      struct PdmaBase * const descriptor = instances[index];
      NM_PDMA_CHANNEL_Type * const entry = &NM_PDMA->CHANNELS[index];
      enum Result res = E_OK;

      if (abtsts & mask)
      {
        res = E_ERROR;
      }
      else if (reqtof & mask)
      {
        /* Disable time-out function, then clear interrupt flag */
        NM_PDMA->TOUTEN &= ~mask;
        NM_PDMA->INTSTS = INTSTS_REQTOF(index);

        res = E_TIMEOUT;
      }
      else if (DSCT_CTL_OPMODE_VALUE(entry->CTL) != OPMODE_IDLE)
      {
        res = E_BUSY;
      }
      else
        res = E_OK;

      descriptor->handler(descriptor, res);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct PdmaBaseConfig * const config = configBase;
  assert(config->channel < ARRAY_SIZE(instances));
  assert(config->event > PDMA_DISABLE && config->event < PDMA_EVENT_END);

  struct PdmaBase * const channel = object;
  const uint32_t offset = config->channel & 0x3;

  channel->reg = NM_PDMA;
  channel->handler = NULL;
  channel->control = 0;
  channel->controller = 0;
  channel->number = config->channel;
  channel->mux.mask = ~REQSEL_CH_MASK(offset);

  if (config->event != PDMA_MEMORY)
  {
    channel->control |= DSCT_CTL_TXTYPE;
    channel->mux.value = REQSEL_CH(offset, config->event);
  }
  else
    channel->mux.value = REQSEL_CH(offset, PDMA_DISABLE);

  if (!sysClockStatus(CLK_PDMA))
  {
    /* Enable clock to peripheral */
    sysClockEnable(CLK_PDMA);
    /* Reset registers to default values */
    sysResetBlock(RST_PDMA);

    /* Clear descriptors */
    for (size_t index = 0; index < CHANNEL_COUNT; ++index)
    {
      NM_PDMA->CHANNELS[index].CTL = 0;
      NM_PDMA->CHANNELS[index].SA = 0;
      NM_PDMA->CHANNELS[index].DA = 0;
      NM_PDMA->CHANNELS[index].NEXT = 0;
    }

    /* Enable interrupts for all channels */
    NM_PDMA->INTEN = INTEN_CH_MASK;
    /* Set base address for a descriptor table */
    NM_PDMA->SCATBA = SCATBA_ADDRESS_TO_BASE((uintptr_t)&_sbss);

    /* Configure interrupts in NVIC */
    irqSetPriority(PDMA_IRQ, CONFIG_PLATFORM_NUMICRO_PDMA_PRIORITY);
    irqEnable(PDMA_IRQ);
  }

  return E_OK;
}
