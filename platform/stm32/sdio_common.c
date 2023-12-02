/*
 * sdio_common.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/sdio_base.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
extern const struct PinGroupEntry sdioPinGroups[];
/*----------------------------------------------------------------------------*/
void sdioConfigPins(struct SdioBase *interface,
    const struct SdioBaseConfig *config)
{
  const PinNumber pinArray[] = {
      config->clk,
      config->cmd,
      config->dat0,
      config->dat1,
      config->dat2,
      config->dat3
  };
  uint8_t width = 4;

  for (size_t index = 0; index < ARRAY_SIZE(pinArray); ++index)
  {
    if (!pinArray[index])
    {
      /* First three pins are mandatory */
      assert(index >= 3);

      width = 1;
      continue;
    }

    const struct PinGroupEntry * const group = pinGroupFind(sdioPinGroups,
        pinArray[index], 0);
    assert(group != NULL);

    const struct Pin pin = pinInit(pinArray[index]);

    pinInput(pin);
    pinSetFunction(pin, group->value);
  }

  interface->width = width;
}
