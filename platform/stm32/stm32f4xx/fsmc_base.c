/*
 * fsmc_base.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/stm32/clocking.h>
#include <halm/platform/stm32/fsmc_base.h>
#include <halm/platform/stm32/fsmc_defs.h>
#include <halm/platform/stm32/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
struct FsmcHandler
{
  struct Entity base;
  struct Entity *memory[4];
};
/*----------------------------------------------------------------------------*/
static bool fsmcHandlerInstantiate(void);
static void fsmcSwitchEnabled(bool);

static enum Result fsmcHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const FsmcHandler =
    &(const struct EntityClass){
    .size = sizeof(struct FsmcHandler),
    .init = fsmcHandlerInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry fsmcAddressPins[] = {
    {
        /* FSMC_A19 to FSMC_A25 */
        .begin = PIN(PORT_E, 2),
        .end = PIN(PORT_E, 6),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_A0 to FSMC_A5 */
        .begin = PIN(PORT_F, 0),
        .end = PIN(PORT_F, 5),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_A6 to FSMC_A9 */
        .begin = PIN(PORT_F, 12),
        .end = PIN(PORT_F, 15),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_A10 to FSMC_A15 */
        .begin = PIN(PORT_G, 0),
        .end = PIN(PORT_G, 5),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_A16 to FSMC_A20 */
        .begin = PIN(PORT_G, 10),
        .end = PIN(PORT_G, 14),
        .channel = 0,
        .value = 12
    }, {
        .begin = 0,
        .end = 0
    }
};

const PinNumber fsmcAddressPinMap[] = {
    PIN(PORT_F, 0),  PIN(PORT_F, 1),  PIN(PORT_F, 2),  PIN(PORT_F, 3),
    PIN(PORT_F, 4),  PIN(PORT_F, 5),  PIN(PORT_F, 12), PIN(PORT_F, 13),
    PIN(PORT_F, 14), PIN(PORT_F, 15), PIN(PORT_G, 0),  PIN(PORT_G, 1),
    PIN(PORT_G, 2),  PIN(PORT_G, 3),  PIN(PORT_G, 4),  PIN(PORT_G, 5),
    PIN(PORT_G, 10), PIN(PORT_G, 11), PIN(PORT_G, 12), PIN(PORT_G, 13),
    PIN(PORT_G, 14), PIN(PORT_E, 5),  PIN(PORT_E, 6),  PIN(PORT_E, 2),
    PIN(PORT_E, 3),  PIN(PORT_E, 4)
};

const struct PinGroupEntry fsmcControlPins[] = {
    {
        /* FSMC_NL */
        .begin = PIN(PORT_B, 7),
        .end = PIN(PORT_B, 7),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_CLK to FSMC_NE1 */
        .begin = PIN(PORT_D, 3),
        .end = PIN(PORT_D, 7),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_NBL0 to FSMC_NBL1 */
        .begin = PIN(PORT_E, 0),
        .end = PIN(PORT_E, 1),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_INTR */
        .begin = PIN(PORT_F, 10),
        .end = PIN(PORT_F, 10),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_INT2 to FSMC_INT3 */
        .begin = PIN(PORT_G, 6),
        .end = PIN(PORT_G, 7),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_NE2 to FSMC_NE3 */
        .begin = PIN(PORT_G, 9),
        .end = PIN(PORT_G, 10),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_NE4 */
        .begin = PIN(PORT_G, 12),
        .end = PIN(PORT_G, 12),
        .channel = 0,
        .value = 12
    }, {
        .begin = 0,
        .end = 0
    }
};

const struct FsmcPinDescription fsmcControlPinMap = {
    .clk = PIN(PORT_D, 3),
    .nl = PIN(PORT_B, 7),
    .noe = PIN(PORT_D, 4),
    .nwe = PIN(PORT_D, 5),
    .nwait = PIN(PORT_D, 6),

    .intr = {
        PIN(PORT_F, 10),
        PIN(PORT_G, 6),
        PIN(PORT_G, 7)
    },
    .nbl = {
        PIN(PORT_E, 0),
        PIN(PORT_E, 1)
    },
    .ne = {
        PIN(PORT_D, 7),
        PIN(PORT_G, 9),
        PIN(PORT_G, 10),
        PIN(PORT_G, 12)
    }
};

const struct PinGroupEntry fsmcDataPins[] = {
    {
        /* FSMC_D2 to FSMC_D3 */
        .begin = PIN(PORT_D, 0),
        .end = PIN(PORT_D, 1),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_D13 to FSMC_D15 */
        .begin = PIN(PORT_D, 8),
        .end = PIN(PORT_D, 10),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_D0 to FSMC_D1 */
        .begin = PIN(PORT_D, 14),
        .end = PIN(PORT_D, 15),
        .channel = 0,
        .value = 12
    }, {
        /* FSMC_D4 to FSMC_D12 */
        .begin = PIN(PORT_E, 7),
        .end = PIN(PORT_E, 15),
        .channel = 0,
        .value = 12
    }, {
        .begin = 0,
        .end = 0
    }
};

const PinNumber fsmcDataPinMap[] = {
    PIN(PORT_D, 14), PIN(PORT_D, 15), PIN(PORT_D, 0),  PIN(PORT_D, 1),
    PIN(PORT_E, 7),  PIN(PORT_E, 8),  PIN(PORT_E, 9),  PIN(PORT_E, 10),
    PIN(PORT_E, 11), PIN(PORT_E, 12), PIN(PORT_E, 13), PIN(PORT_E, 14),
    PIN(PORT_E, 15), PIN(PORT_D, 8),  PIN(PORT_D, 9),  PIN(PORT_D, 10)
};
/*----------------------------------------------------------------------------*/
static struct FsmcHandler *fsmcHandler = NULL;
/*----------------------------------------------------------------------------*/
uint32_t fsmcGetClock(void)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void *fsmcGetMemoryAddress(uint8_t bank, uint8_t subbank)
{
  assert(bank < 4);

  switch (bank)
  {
    case 0:
      return (void *)(STM_FSMC_BANK1_BASE + FSMC_SUBBANK_SIZE * subbank);

    case 1:
      return (void *)STM_FSMC_BANK2_BASE;

    case 2:
      return (void *)STM_FSMC_BANK3_BASE;

    case 3:
      return (void *)STM_FSMC_BANK4_BASE;

    default:
      return NULL;
  }
}
/*----------------------------------------------------------------------------*/
bool fsmcSetMemoryDescriptor(uint8_t bank, const struct Entity *current,
    struct Entity *memory)
{
  assert(bank < ARRAY_SIZE(fsmcHandler->memory));

  if (fsmcHandlerInstantiate())
  {
    if (compareExchangePointer(&fsmcHandler->memory[bank], &current, memory))
    {
      fsmcSwitchEnabled(memory != NULL);
      return true;
    }
  }

  return false;
}
/*----------------------------------------------------------------------------*/
static bool fsmcHandlerInstantiate(void)
{
  if (fsmcHandler == NULL)
    fsmcHandler = init(FsmcHandler, NULL);

  return fsmcHandler != NULL;
}
/*----------------------------------------------------------------------------*/
static void fsmcSwitchEnabled(bool state)
{
  if (state)
  {
    /* Enable clocks to register memory and peripheral */
    sysClockEnable(CLK_FSMC);
    /* Reset registers to their default values */
    sysResetPulse(RST_FSMC);
  }
  else if (!state)
  {
    bool allChannelsDisabled = true;

    for (size_t index = 0; index < ARRAY_SIZE(fsmcHandler->memory); ++index)
    {
      if (fsmcHandler->memory[index] != NULL)
      {
        allChannelsDisabled = false;
        break;
      }
    }

    if (allChannelsDisabled)
    {
      /* Disable clocks */
      sysClockDisable(CLK_FSMC);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result fsmcHandlerInit(void *object, const void *)
{
  struct FsmcHandler * const handler = object;

  for (size_t channel = 0; channel < ARRAY_SIZE(handler->memory); ++channel)
    handler->memory[channel] = NULL;

  return E_OK;
}
