/*
 * sdio_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/bdma_oneshot.h>
#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/sdio_base.h>
#include <halm/platform/stm32/system.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SdioBase *);
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *, const void *);
static void sdioDeinit(void *);
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SdioBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = sdioInit,
    .deinit = sdioDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry sdioPinGroups[] = {
    {
        /* SDIO_D4 .. SDIO_D5 */
        .begin = PIN(PORT_B, 8),
        .end = PIN(PORT_B, 9),
        .channel = 0,
        .value = 0
    }, {
        /* SDIO_D6 .. SDIO_D7, SDIO_D0 .. SDIO_D3 and SDIO_CK */
        .begin = PIN(PORT_C, 6),
        .end = PIN(PORT_C, 12),
        .channel = 0,
        .value = 0
    }, {
        /* SDIO_CMD */
        .begin = PIN(PORT_D, 2),
        .end = PIN(PORT_D, 2),
        .channel = 0,
        .value = 0
    }, {
        /* End of pin function association list */
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct SdioBase *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SdioBase *object)
{
  if (instance == NULL)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void SDIO_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint32_t sdioGetClock(const struct SdioBase *)
{
  return clockFrequency(SystemClock);
}
/*----------------------------------------------------------------------------*/
void *sdioMakeOneShotDma(uint8_t stream, enum DmaPriority priority,
    enum DmaType type)
{
  const struct BdmaOneShotConfig config = {
      .event = DMA_GENERIC,
      .priority = priority,
      .type = type,
      .stream = stream
  };

  return init(BdmaOneShot, &config);
}
/*----------------------------------------------------------------------------*/
static enum Result sdioInit(void *object, const void *configBase)
{
  const struct SdioBaseConfig * const config = configBase;
  struct SdioBase * const interface = object;

  if (!setInstance(interface))
    return E_BUSY;

  /* Configure input and output pins */
  sdioConfigPins(interface, config);
  /* Enable clocks to register interface and peripheral */
  sysClockEnable(CLK_SDIO);

  interface->handler = NULL;
  interface->irq = SDIO_IRQ;
  interface->reg = STM_SDIO;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *)
{
  sysClockDisable(CLK_SDIO);
  instance = NULL;
}
