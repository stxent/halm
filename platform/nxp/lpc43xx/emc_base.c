/*
 * emc_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/irq.h>
#include <halm/platform/nxp/emc_base.h>
#include <halm/platform/nxp/emc_defs.h>
#include <halm/platform/nxp/lpc43xx/clocking.h>
#include <halm/platform/nxp/lpc43xx/system.h>
#include <xcore/bits.h>
#include <assert.h>
#include <stddef.h>
/*----------------------------------------------------------------------------*/
struct EmcHandler
{
  struct Entity base;

  struct Entity *dm[4];
  struct Entity *sm[4];
};
/*----------------------------------------------------------------------------*/
static bool emcHandlerInstantiate(void);
static void emcSwitchEnabled(bool);

static enum Result emcHandlerInit(void *, const void *);
/*----------------------------------------------------------------------------*/
static const struct EntityClass * const EmcHandler =
    &(const struct EntityClass){
    .size = sizeof(struct EmcHandler),
    .init = emcHandlerInit,
    .deinit = deletedDestructorTrap
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry emcAddressPins[] = {
    {
        /* EMC_A5:EMC_A7 are mapped to P1_0:P1_2 */
        .begin = PIN(PORT_1, 0),
        .end = PIN(PORT_1, 2),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_A13:EMC_A11 are mapped to P2_0:P2_2 */
        .begin = PIN(PORT_2, 0),
        .end = PIN(PORT_2, 2),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_A10 is mapped to P2_6 */
        .begin = PIN(PORT_2, 6),
        .end = PIN(PORT_2, 6),
        .channel = 0,
        .value = 2
    }, {
        /*
         * EMC_A9:EMC_A8 are mapped to P2_7:P2_8
         * EMC_A0:EMC_A4 are mapped to P2_9:P2_13
         */
        .begin = PIN(PORT_2, 7),
        .end = PIN(PORT_2, 13),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_A15:EMC_A14 are mapped to P6_7:P6_8 */
        .begin = PIN(PORT_6, 7),
        .end = PIN(PORT_6, 8),
        .channel = 0,
        .value = 1
    }, {
        /* EMC_A23 is mapped to PA_4 */
        .begin = PIN(PORT_A, 4),
        .end = PIN(PORT_A, 4),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_A17:EMC_A16 are mapped to PD_15:PD_16 */
        .begin = PIN(PORT_D, 15),
        .end = PIN(PORT_D, 16),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_A18:EMC_A22 are mapped to PE_0:PE_4 */
        .begin = PIN(PORT_E, 0),
        .end = PIN(PORT_E, 4),
        .channel = 0,
        .value = 3
    }, {
        .begin = 0,
        .end = 0
    }
};

const PinNumber emcAddressPinMap[] = {
    PIN(PORT_2, 9),  PIN(PORT_2, 10), PIN(PORT_2, 11), PIN(PORT_2, 12),
    PIN(PORT_2, 13), PIN(PORT_1, 0),  PIN(PORT_1, 1),  PIN(PORT_1, 2),
    PIN(PORT_2, 8),  PIN(PORT_2, 7),  PIN(PORT_2, 6),  PIN(PORT_2, 2),
    PIN(PORT_2, 1),  PIN(PORT_2, 0),  PIN(PORT_6, 8),  PIN(PORT_6, 7),
    PIN(PORT_D, 16), PIN(PORT_D, 15), PIN(PORT_E, 0),  PIN(PORT_E, 1),
    PIN(PORT_E, 2),  PIN(PORT_E, 3),  PIN(PORT_E, 4),  PIN(PORT_A, 4)
};

const struct PinGroupEntry emcControlPins[] = {
    {
        /* EMC_OE, EMC_BLS0, EMC_CS0, EMC_WE */
        .begin = PIN(PORT_1, 3),
        .end = PIN(PORT_1, 6),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_DYCS1, EMC_CKEOUT1 */
        .begin = PIN(PORT_6, 1),
        .end = PIN(PORT_6, 2),
        .channel = 0,
        .value = 1
    }, {
        /* EMC_CS1, EMC_CAS, EMC_RAS */
        .begin = PIN(PORT_6, 3),
        .end = PIN(PORT_6, 5),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_BLS1 */
        .begin = PIN(PORT_6, 6),
        .end = PIN(PORT_6, 6),
        .channel = 0,
        .value = 1
    }, {
        /* EMC_DYCS0, EMC_DQMOUT1, EMC_CKEOUT0, EMC_DQMOUT0 */
        .begin = PIN(PORT_6, 9),
        .end = PIN(PORT_6, 12),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_DQMOUT2, EMC_CKEOUT2 */
        .begin = PIN(PORT_D, 0),
        .end = PIN(PORT_D, 1),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_BLS3, EMC_CS3, EMC_CS2, EMC_BLS2, EMC_DYCS2 */
        .begin = PIN(PORT_D, 10),
        .end = PIN(PORT_D, 14),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_DQMOUT3, EMC_DYCS3, EMC_CKEOUT3 */
        .begin = PIN(PORT_E, 13),
        .end = PIN(PORT_E, 15),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_CLK0, EMC_CLK1, EMC_CLK2, EMC_CLK3 */
        .begin = PIN(PORT_CLK, 0),
        .end = PIN(PORT_CLK, 3),
        .channel = 0,
        .value = 0
    }, {
        .begin = 0,
        .end = 0
    }
};

const struct EmcPinDescription emcControlPinMap = {
    .cas = PIN(PORT_6, 4),
    .oe  = PIN(PORT_1, 3),
    .ras = PIN(PORT_6, 5),
    .we  = PIN(PORT_1, 6),

    .bls = {
        PIN(PORT_1, 4),
        PIN(PORT_6, 6),
        PIN(PORT_D, 13),
        PIN(PORT_D, 10)
    },
    .ckeout = {
        PIN(PORT_6, 11),
        PIN(PORT_6, 2),
        PIN(PORT_D, 1),
        PIN(PORT_E, 15)
    },
    .clk = {
        PIN(PORT_CLK, 0),
        PIN(PORT_CLK, 1),
        PIN(PORT_CLK, 2),
        PIN(PORT_CLK, 3)
    },
    .cs = {
        PIN(PORT_1, 5),
        PIN(PORT_6, 3),
        PIN(PORT_D, 12),
        PIN(PORT_D, 11)
    },
    .dqmout = {
        PIN(PORT_6, 12),
        PIN(PORT_6, 10),
        PIN(PORT_D, 0),
        PIN(PORT_E, 13)
    },
    .dycs = {
        PIN(PORT_6, 9),
        PIN(PORT_6, 1),
        PIN(PORT_D, 14),
        PIN(PORT_E, 14)
    }
};

const struct PinGroupEntry emcDataPins[] = {
    {
        /* EMC_D0:EMC_D7 are mapped to P1_7:P1_14 */
        .begin = PIN(PORT_1, 7),
        .end = PIN(PORT_1, 14),
        .channel = 0,
        .value = 3
    }, {
        /*
         * EMC_D8:EMC_D11 are mapped to P5_4:P5_7
         * EMC_D12:EMC_D15 are mapped to P5_0:P5_3
         */
        .begin = PIN(PORT_5, 0),
        .end = PIN(PORT_5, 7),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_D16:EMC_D23 are mapped to PD_2:PD_9 */
        .begin = PIN(PORT_D, 2),
        .end = PIN(PORT_D, 9),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_D24:EMC_D31 are mapped to PE_5:PE_12 */
        .begin = PIN(PORT_1, 5),
        .end = PIN(PORT_1, 6),
        .channel = 0,
        .value = 3
    }, {
        .begin = 0,
        .end = 0
    }
};

const PinNumber emcDataPinMap[] = {
    PIN(PORT_1, 7),  PIN(PORT_1, 8),  PIN(PORT_1, 9),  PIN(PORT_1, 10),
    PIN(PORT_1, 11), PIN(PORT_1, 12), PIN(PORT_1, 13), PIN(PORT_1, 14),
    PIN(PORT_5, 4),  PIN(PORT_5, 5),  PIN(PORT_5, 6),  PIN(PORT_5, 7),
    PIN(PORT_5, 0),  PIN(PORT_5, 1),  PIN(PORT_5, 2),  PIN(PORT_5, 3),
    PIN(PORT_D, 2),  PIN(PORT_D, 3),  PIN(PORT_D, 4),  PIN(PORT_D, 5),
    PIN(PORT_D, 6),  PIN(PORT_D, 7),  PIN(PORT_D, 8),  PIN(PORT_D, 9),
    PIN(PORT_E, 5),  PIN(PORT_E, 6),  PIN(PORT_E, 7),  PIN(PORT_E, 8),
    PIN(PORT_E, 9),  PIN(PORT_E, 10), PIN(PORT_E, 11), PIN(PORT_E, 12)
};
/*----------------------------------------------------------------------------*/
static struct EmcHandler *emcHandler = 0;
/*----------------------------------------------------------------------------*/
uint32_t emcGetClock(void)
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
void *emcGetDynamicMemoryAddress(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(emcHandler->dm));

  switch (channel)
  {
    case 0:
      return (void *)LPC_EMC_DYCS0_BASE;

    case 1:
      return (void *)LPC_EMC_DYCS1_BASE;

    case 2:
      return (void *)LPC_EMC_DYCS2_BASE;

    case 3:
      return (void *)LPC_EMC_DYCS3_BASE;

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
void *emcGetStaticMemoryAddress(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(emcHandler->sm));

  switch (channel)
  {
    case 0:
      return (void *)LPC_EMC_CS0_BASE;

    case 1:
      return (void *)LPC_EMC_CS1_BASE;

    case 2:
      return (void *)LPC_EMC_CS2_BASE;

    case 3:
      return (void *)LPC_EMC_CS3_BASE;

    default:
      return 0;
  }
}
/*----------------------------------------------------------------------------*/
bool emcSetDynamicMemoryDescriptor(uint8_t channel,
    const struct Entity *current, struct Entity *memory)
{
  bool completed = false;

  if (emcHandlerInstantiate())
  {
    assert(channel < ARRAY_SIZE(emcHandler->dm));

    completed = emcHandler->dm[channel] == current;

    if (completed)
    {
      emcHandler->dm[channel] = memory;
      emcSwitchEnabled(memory != 0);
    }
  }

  return completed;
}
/*----------------------------------------------------------------------------*/
bool emcSetStaticMemoryDescriptor(uint8_t channel,
    const struct Entity *current, struct Entity *memory)
{
  bool completed = false;

  if (emcHandlerInstantiate())
  {
    assert(channel < ARRAY_SIZE(emcHandler->sm));

    completed = emcHandler->sm[channel] == current;

    if (completed)
    {
      emcHandler->sm[channel] = memory;
      emcSwitchEnabled(memory != 0);
    }
  }

  return completed;
}
/*----------------------------------------------------------------------------*/
static bool emcHandlerInstantiate(void)
{
  if (!emcHandler)
    emcHandler = init(EmcHandler, 0);

  return emcHandler != 0;
}
/*----------------------------------------------------------------------------*/
static void emcSwitchEnabled(bool state)
{
  if (state && !(LPC_EMC->CONTROL & CONTROL_E))
  {
    /* Enable clocks to register memory and peripheral */
    sysClockEnable(CLK_M4_EMC);
    sysClockEnable(CLK_M4_EMCDIV);
    /* Reset registers to their default values */
    sysResetEnable(RST_EMC);

    /* Enable peripheral */
    LPC_EMC->CONTROL = CONTROL_E;
  }
  else if (!state && (LPC_EMC->CONTROL & CONTROL_E))
  {
    bool allChannelsDisabled = true;

    for (size_t channel = 0; channel < ARRAY_SIZE(emcHandler->dm); ++channel)
    {
      if (emcHandler->dm[channel])
      {
        allChannelsDisabled = false;
        break;
      }
    }

    for (size_t channel = 0; channel < ARRAY_SIZE(emcHandler->sm); ++channel)
    {
      if (emcHandler->sm[channel])
      {
        allChannelsDisabled = false;
        break;
      }
    }

    if (allChannelsDisabled)
    {
      /* Disable peripheral */
      LPC_EMC->CONTROL &= ~CONTROL_E;

      /* Disable clocks */
      sysClockDisable(CLK_M4_EMCDIV);
      sysClockDisable(CLK_M4_EMC);
    }
  }
}
/*----------------------------------------------------------------------------*/
static enum Result emcHandlerInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct EmcHandler * const handler = object;

  for (size_t channel = 0; channel < ARRAY_SIZE(handler->dm); ++channel)
    handler->dm[channel] = 0;
  for (size_t channel = 0; channel < ARRAY_SIZE(handler->sm); ++channel)
    handler->sm[channel] = 0;

  return E_OK;
}
