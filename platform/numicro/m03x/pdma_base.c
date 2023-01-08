/*
 * pdma_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/irq.h>
#include <halm/platform/platform_defs.h>
#include <halm/platform/numicro/m03x/pdma_defs.h>
#include <halm/platform/numicro/pdma_base.h>
#include <halm/platform/numicro/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *, const void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const PdmaBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = channelInit,
    .deinit = 0 /* Default destructor */
};
/*----------------------------------------------------------------------------*/
static struct PdmaBase *instances[9] = {0};
/*----------------------------------------------------------------------------*/
const struct PdmaBase *pdmaGetInstance(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(instances));
  return instances[channel];
}
/*----------------------------------------------------------------------------*/
void pdmaResetInstance(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(instances));
  instances[channel] = 0;
}
/*----------------------------------------------------------------------------*/
bool pdmaSetInstance(uint8_t channel, struct PdmaBase *object)
{
  assert(object);
  assert(channel < ARRAY_SIZE(instances));

  void *expected = 0;
  return compareExchangePointer(&instances[channel], &expected, object);
}
/*----------------------------------------------------------------------------*/
void pdmaSetMux(struct PdmaBase *descriptor)
{
  volatile uint32_t * const reg = &NM_PDMA->REQSEL[descriptor->index >> 2];
  *reg = (*reg & descriptor->mux.mask) | descriptor->mux.value;
}
/*----------------------------------------------------------------------------*/
void PDMA_ISR(void)
{
  const uint32_t abtsts = NM_PDMA->ABTSTS;
  const uint32_t tdsts = NM_PDMA->TDSTS;
  const uint32_t event = abtsts | tdsts;

  NM_PDMA->ABTSTS = abtsts;
  NM_PDMA->TDSTS = tdsts;

  for (size_t index = ARRAY_SIZE(instances); index > 0; --index)
  {
    const size_t channel = index - 1;
    struct PdmaBase * const descriptor = instances[channel];
    const uint32_t mask = 1 << channel;
    enum Result res = E_OK;

    if (event & mask)
    {
      if (abtsts & mask)
        res = E_ERROR;

      descriptor->handler(descriptor, res);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result channelInit(void *object, const void *configBase)
{
  const struct PdmaBaseConfig * const config = configBase;
  struct PdmaBase * const channel = object;

  assert(config->channel < ARRAY_SIZE(instances));
  assert(config->controller == 0);
  assert(config->event > PDMA_DISABLE && config->event < PDMA_EVENT_END);

  const uint32_t offset = config->channel & 0x3;

  channel->reg = NM_PDMA;
  channel->handler = 0;
  channel->control = 0;
  channel->index = config->channel;
  channel->mux.mask = ~REQSEL_CH_MASK(offset);

  if (config->event != PDMA_MEMORY)
  {
    channel->control |= DSCT_CTL_TXTYPE;
    channel->mux.value = REQSEL_CH_VALUE(offset, config->event);
  }
  else
    channel->mux.value = REQSEL_CH_VALUE(offset, PDMA_DISABLE);

  if (!sysClockStatus(CLK_PDMA))
  {
    /* Enable clock to peripheral */
    sysClockEnable(CLK_PDMA);
    /* Reset registers to default values */
    sysResetBlock(RST_PDMA);

    /* Configure interrupts */
    irqSetPriority(PDMA_IRQ, CONFIG_PLATFORM_NUMICRO_PDMA_PRIORITY);
    irqEnable(PDMA_IRQ);
  }

  return E_OK;
}
