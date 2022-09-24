/*
 * sgpio_base.c
 * Copyright (C) 2021 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/clocking.h>
#include <halm/platform/lpc/sgpio_base.h>
#include <halm/platform/lpc/sgpio_defs.h>
#include <halm/platform/lpc/system.h>
#include <halm/platform/platform_defs.h>
/*----------------------------------------------------------------------------*/
#define PACK_VALUE(function, number)  (((number) << 4) | (function))
#define UNPACK_FUNCTION(value)        ((value) & 0x0F)
#define UNPACK_NUMBER(value)          (((value) >> 4) & 0x0F)
/*----------------------------------------------------------------------------*/
struct PinDescriptor
{
  uint8_t number;
  uint8_t port;
};
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SgpioBase *);
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *, const void *);

#ifndef CONFIG_PLATFORM_LPC_SGPIO_NO_DEINIT
static void unitDeinit(void *);
#else
#define unitDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const SgpioBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry sgpioPins[] = {
    {
        /* SGPIO0:SGPIO1 are mapped to P0_0:P0_1 */
        .begin = PIN(PORT_0, 0),
        .end = PIN(PORT_0, 1),
        .channel = 0,
        .value = PACK_VALUE(3, 0)
    }, {
        /* SGPIO7 is mapped to P1_0 */
        .begin = PIN(PORT_1, 0),
        .end = PIN(PORT_1, 0),
        .channel = 0,
        .value = PACK_VALUE(6, 7)
    }, {
        /* SGPIO8:SGPIO9 are mapped to P1_1:P1_2 */
        .begin = PIN(PORT_1, 1),
        .end = PIN(PORT_1, 2),
        .channel = 0,
        .value = PACK_VALUE(3, 8)
    }, {
        /* SGPIO10:SGPIO11 are mapped to P1_3:P1_4 */
        .begin = PIN(PORT_1, 3),
        .end = PIN(PORT_1, 4),
        .channel = 0,
        .value = PACK_VALUE(2, 10)
    }, {
        /* SGPIO15 is mapped to P1_5 */
        .begin = PIN(PORT_1, 5),
        .end = PIN(PORT_1, 5),
        .channel = 0,
        .value = PACK_VALUE(6, 15)
    }, {
        /* SGPIO14 is mapped to P1_6 */
        .begin = PIN(PORT_1, 6),
        .end = PIN(PORT_1, 6),
        .channel = 0,
        .value = PACK_VALUE(6, 14)
    }, {
        /* SGPIO8:SGPIO10 are mapped to P1_12:P1_14 */
        .begin = PIN(PORT_1, 12),
        .end = PIN(PORT_1, 14),
        .channel = 0,
        .value = PACK_VALUE(6, 8)
    }, {
        /* SGPIO2:SGPIO3 are mapped to P1_15:P1_16 */
        .begin = PIN(PORT_1, 15),
        .end = PIN(PORT_1, 16),
        .channel = 0,
        .value = PACK_VALUE(2, 2)
    }, {
        /* SGPIO11:SGPIO12 are mapped to P1_17:P1_18 */
        .begin = PIN(PORT_1, 17),
        .end = PIN(PORT_1, 18),
        .channel = 0,
        .value = PACK_VALUE(6, 11)
    }, {
        /* SGPIO13 is mapped to P1_20 */
        .begin = PIN(PORT_1, 20),
        .end = PIN(PORT_1, 20),
        .channel = 0,
        .value = PACK_VALUE(6, 13)
    }, {
        /* SGPIO4:SGPIO6 are mapped to P2_0:P2_2 */
        .begin = PIN(PORT_2, 0),
        .end = PIN(PORT_2, 2),
        .channel = 0,
        .value = PACK_VALUE(0, 4)
    }, {
        /* SGPIO12:SGPIO14 are mapped to P2_3:P2_5 */
        .begin = PIN(PORT_2, 3),
        .end = PIN(PORT_2, 5),
        .channel = 0,
        .value = PACK_VALUE(0, 12)
    }, {
        /* SGPIO7 is mapped to P2_6 */
        .begin = PIN(PORT_2, 6),
        .end = PIN(PORT_2, 6),
        .channel = 0,
        .value = PACK_VALUE(0, 7)
    }, {
        /* SGPIO15 is mapped to P2_8 */
        .begin = PIN(PORT_2, 8),
        .end = PIN(PORT_2, 8),
        .channel = 0,
        .value = PACK_VALUE(0, 15)
    }, {
        /* SGPIO8:SGPIO12 are mapped to P4_2:P4_6 */
        .begin = PIN(PORT_4, 2),
        .end = PIN(PORT_4, 6),
        .channel = 0,
        .value = PACK_VALUE(7, 8)
    }, {
        /* SGPIO13:SGPIO15 are mapped to P4_8:P4_10 */
        .begin = PIN(PORT_4, 8),
        .end = PIN(PORT_4, 10),
        .channel = 0,
        .value = PACK_VALUE(7, 13)
    }, {
        /* SGPIO4 is mapped to P6_3 */
        .begin = PIN(PORT_6, 3),
        .end = PIN(PORT_6, 3),
        .channel = 0,
        .value = PACK_VALUE(2, 4)
    }, {
        /* SGPIO5:SGPIO7 are mapped to P6_6:P6_8 */
        .begin = PIN(PORT_6, 6),
        .end = PIN(PORT_6, 8),
        .channel = 0,
        .value = PACK_VALUE(2, 5)
    }, {
        /* SGPIO4:SGPIO6 are mapped to P7_0:P7_2 */
        .begin = PIN(PORT_7, 0),
        .end = PIN(PORT_7, 2),
        .channel = 0,
        .value = PACK_VALUE(7, 4)
    }, {
        /* SGPIO7 is mapped to P7_7 */
        .begin = PIN(PORT_7, 7),
        .end = PIN(PORT_7, 7),
        .channel = 0,
        .value = PACK_VALUE(7, 7)
    }, {
        /* SGPIO8:SGPIO10 are mapped to P8_0:P8_2 */
        .begin = PIN(PORT_8, 0),
        .end = PIN(PORT_8, 2),
        .channel = 0,
        .value = PACK_VALUE(4, 8)
    }, {
        /* SGPIO0:SGPIO2 are mapped to P9_0:P9_2 */
        .begin = PIN(PORT_9, 0),
        .end = PIN(PORT_9, 2),
        .channel = 0,
        .value = PACK_VALUE(6, 0)
    }, {
        /* SGPIO9 is mapped to P9_3 */
        .begin = PIN(PORT_9, 3),
        .end = PIN(PORT_9, 3),
        .channel = 0,
        .value = PACK_VALUE(6, 9)
    }, {
        /* SGPIO4 is mapped to P9_4 */
        .begin = PIN(PORT_9, 4),
        .end = PIN(PORT_9, 4),
        .channel = 0,
        .value = PACK_VALUE(6, 4)
    }, {
        /* SGPIO3 is mapped to P9_5 */
        .begin = PIN(PORT_9, 5),
        .end = PIN(PORT_9, 5),
        .channel = 0,
        .value = PACK_VALUE(6, 3)
    }, {
        /* SGPIO8 is mapped to P9_6 */
        .begin = PIN(PORT_9, 6),
        .end = PIN(PORT_9, 6),
        .channel = 0,
        .value = PACK_VALUE(6, 8)
    }, {
        /* SGPIO11:SGPIO13 are mapped to PC_12:PC_14 */
        .begin = PIN(PORT_C, 12),
        .end = PIN(PORT_C, 14),
        .channel = 0,
        .value = PACK_VALUE(5, 11)
    }, {
        /* SGPIO4:SGPIO13 are mapped to PD_0:PD_9 */
        .begin = PIN(PORT_D, 0),
        .end = PIN(PORT_D, 9),
        .channel = 0,
        .value = PACK_VALUE(7, 4)
    }, {
        /* SGPIO0:SGPIO2 are mapped to PF_1:PF_3 */
        .begin = PIN(PORT_F, 1),
        .end = PIN(PORT_F, 3),
        .channel = 0,
        .value = PACK_VALUE(6, 0)
    }, {
        /* SGPIO4:SGPIO7 are mapped to PF_5:PF_8 */
        .begin = PIN(PORT_F, 5),
        .end = PIN(PORT_F, 8),
        .channel = 0,
        .value = PACK_VALUE(6, 4)
    }, {
        /* SGPIO3 is mapped to PF_9 */
        .begin = PIN(PORT_F, 9),
        .end = PIN(PORT_F, 9),
        .channel = 0,
        .value = PACK_VALUE(6, 3)
    }, {
        .begin = 0,
        .end = 0
    }
};
/*----------------------------------------------------------------------------*/
static struct SgpioBase *instance = 0;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct SgpioBase *object)
{
  if (!instance)
  {
    instance = object;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
void SGPIO_ISR(void)
{
  instance->handler(instance);
}
/*----------------------------------------------------------------------------*/
uint8_t sgpioConfigPin(PinNumber key, enum PinPull pull)
{
  const struct PinGroupEntry * const group = pinGroupFind(sgpioPins, key, 0);
  assert(group);

  const uint8_t offset = PIN_TO_OFFSET(key) - PIN_TO_OFFSET(group->begin);
  const struct Pin pin = pinInit(key);

  pinInput(pin);
  pinSetFunction(pin, UNPACK_FUNCTION(group->value));
  pinSetPull(pin, pull);

  return offset + UNPACK_NUMBER(group->value);
}
/*----------------------------------------------------------------------------*/
uint32_t sgpioGetClock(const struct SgpioBase *unit __attribute__((unused)))
{
  return clockFrequency(MainClock);
}
/*----------------------------------------------------------------------------*/
enum SgpioSlice sgpioPinToSlice(enum SgpioPin pin, uint8_t mux)
{
  if (mux == 0x0)
  {
    /* 1-bit */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_A, SGPIO_SLICE_I, SGPIO_SLICE_E, SGPIO_SLICE_J,
        SGPIO_SLICE_C, SGPIO_SLICE_K, SGPIO_SLICE_F, SGPIO_SLICE_L,
        SGPIO_SLICE_B, SGPIO_SLICE_M, SGPIO_SLICE_G, SGPIO_SLICE_N,
        SGPIO_SLICE_D, SGPIO_SLICE_O, SGPIO_SLICE_H, SGPIO_SLICE_P
    };

    return MUX_TABLE[pin];
  }
  else if (mux == 0x1)
  {
    /* Clock */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_B, SGPIO_SLICE_D, SGPIO_SLICE_E, SGPIO_SLICE_H,
        SGPIO_SLICE_C, SGPIO_SLICE_F, SGPIO_SLICE_O, SGPIO_SLICE_P,
        SGPIO_SLICE_A, SGPIO_SLICE_M, SGPIO_SLICE_G, SGPIO_SLICE_N,
        SGPIO_SLICE_I, SGPIO_SLICE_J, SGPIO_SLICE_K, SGPIO_SLICE_L
    };

    return MUX_TABLE[pin];
  }
  else if (mux == 0xB)
  {
    /* 8-bit 8c */
    return pin < SGPIO_8 ? SGPIO_SLICE_L : SGPIO_SLICE_N;
  }
  else if (mux == 0xA)
  {
    /* 8-bit 8b */
    return pin < SGPIO_8 ? SGPIO_SLICE_J : SGPIO_SLICE_M;
  }
  else if (mux == 0x9)
  {
    /* 8-bit 8a */
    return pin < SGPIO_8 ? SGPIO_SLICE_A : SGPIO_SLICE_B;
  }
  else if (mux == 0x7)
  {
    /* 4-bit 4c */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_J, SGPIO_SLICE_L, SGPIO_SLICE_N, SGPIO_SLICE_P
    };

    return MUX_TABLE[pin >> 2];
  }
  else if (mux == 0x6)
  {
    /* 4-bit 4b */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_I, SGPIO_SLICE_K, SGPIO_SLICE_M, SGPIO_SLICE_O
    };

    return MUX_TABLE[pin >> 2];
  }
  else if (mux == 0x5)
  {
    /* 4-bit 4a */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_A, SGPIO_SLICE_C, SGPIO_SLICE_B, SGPIO_SLICE_D
    };

    return MUX_TABLE[pin >> 2];
  }
  else if (mux == 0x3)
  {
    /* 2-bit 2c */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_J, SGPIO_SLICE_I, SGPIO_SLICE_L, SGPIO_SLICE_K,
        SGPIO_SLICE_N, SGPIO_SLICE_M, SGPIO_SLICE_P, SGPIO_SLICE_O
    };

    return MUX_TABLE[pin >> 1];
  }
  else if (mux == 0x2)
  {
    /* 2-bit 2b */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_I, SGPIO_SLICE_J, SGPIO_SLICE_K, SGPIO_SLICE_L,
        SGPIO_SLICE_M, SGPIO_SLICE_N, SGPIO_SLICE_O, SGPIO_SLICE_P
    };

    return MUX_TABLE[pin >> 1];
  }
  else if (mux == 0x1)
  {
    /* 2-bit 2a */
    static const enum SgpioSlice MUX_TABLE[] = {
        SGPIO_SLICE_A, SGPIO_SLICE_E, SGPIO_SLICE_C, SGPIO_SLICE_F,
        SGPIO_SLICE_B, SGPIO_SLICE_G, SGPIO_SLICE_D, SGPIO_SLICE_H
    };

    return MUX_TABLE[pin >> 1];
  }
  else
  {
    /* GPIO */
    return (enum SgpioSlice)pin;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result unitInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct SgpioBase * const unit = object;

  if (!setInstance(unit))
    return E_BUSY;

  unit->handler = 0;
  unit->irq = SGPIO_IRQ;
  unit->reg = LPC_SGPIO;

  /* Enable clock to peripheral */
  sysClockEnable(CLK_PERIPH_SGPIO);
  /* Reset registers to default values */
  sysResetEnable(RST_SGPIO);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_LPC_SCT_NO_DEINIT
static void unitDeinit(void *object __attribute__((unused)))
{
  sysClockDisable(CLK_PERIPH_SGPIO);
}
#endif
