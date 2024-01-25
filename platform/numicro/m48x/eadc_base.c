/*
 * eadc_base.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/clocking.h>
#include <halm/platform/numicro/eadc_base.h>
#include <halm/platform/numicro/eadc_defs.h>
#include <halm/platform/numicro/system.h>
#include <xcore/atomic.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
#define MAX_FREQUENCY 72000000
/*----------------------------------------------------------------------------*/
void adcBaseHandler0(void) __attribute__((weak));
void adcBaseHandler1(void) __attribute__((weak));

static enum Result adcInit(void *, const void *);

#ifndef CONFIG_PLATFORM_NUMICRO_EADC_NO_DEINIT
static void adcDeinit(void *);
#else
#  define adcDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct EntityClass * const EadcBase = &(const struct EntityClass){
    .size = 0, /* Abstract class */
    .init = adcInit,
    .deinit = adcDeinit
};
/*----------------------------------------------------------------------------*/
const struct PinEntry eadcPins[] = {
#ifdef CONFIG_PLATFORM_NUMICRO_EADC0
    {
        .key = PIN(PORT_B, 0), /* EADC0_CH0 */
        .channel = 0,
        .value = 0
    }, {
        .key = PIN(PORT_B, 1), /* EADC0_CH1 */
        .channel = 0,
        .value = 1
    }, {
        .key = PIN(PORT_B, 2), /* EADC0_CH2 */
        .channel = 0,
        .value = 2
    }, {
        .key = PIN(PORT_B, 3), /* EADC0_CH3 */
        .channel = 0,
        .value = 3
    }, {
        .key = PIN(PORT_B, 4), /* EADC0_CH4 */
        .channel = 0,
        .value = 4
    }, {
        .key = PIN(PORT_B, 5), /* EADC0_CH5 */
        .channel = 0,
        .value = 5
    }, {
        .key = PIN(PORT_B, 6), /* EADC0_CH6 */
        .channel = 0,
        .value = 6
    }, {
        .key = PIN(PORT_B, 7), /* EADC0_CH7 */
        .channel = 0,
        .value = 7
    }, {
        .key = PIN(PORT_B, 8), /* EADC0_CH8 */
        .channel = 0,
        .value = 8
    }, {
        .key = PIN(PORT_B, 9), /* EADC0_CH9 */
        .channel = 0,
        .value = 9
    }, {
        .key = PIN(PORT_B, 10), /* EADC0_CH10 */
        .channel = 0,
        .value = 10
    }, {
        .key = PIN(PORT_B, 11), /* EADC0_CH11 */
        .channel = 0,
        .value = 11
    }, {
        .key = PIN(PORT_B, 12), /* EADC0_CH12 */
        .channel = 0,
        .value = 12
    }, {
        .key = PIN(PORT_B, 13), /* EADC0_CH13 */
        .channel = 0,
        .value = 13
    }, {
        .key = PIN(PORT_B, 14), /* EADC0_CH14 */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_B, 15), /* EADC0_CH15 */
        .channel = 0,
        .value = 15
    }, {
        .key = PIN(PORT_C, 1), /* EADC0_ST */
        .channel = 0,
        .value = 15
    }, {
        .key = PIN(PORT_C, 13), /* EADC0_ST */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_D, 12), /* EADC0_ST */
        .channel = 0,
        .value = 14
    }, {
        .key = PIN(PORT_F, 5), /* EADC0_ST */
        .channel = 0,
        .value = 11
    },
#endif
#ifdef CONFIG_PLATFORM_NUMICRO_EADC1
    {
        .key = PIN(PORT_A, 8), /* EADC1_CH4 */
        .channel = 1,
        .value = 4
    }, {
        .key = PIN(PORT_A, 9), /* EADC1_CH5 */
        .channel = 1,
        .value = 5
    }, {
        .key = PIN(PORT_A, 10), /* EADC1_CH6 */
        .channel = 1,
        .value = 6
    }, {
        .key = PIN(PORT_A, 11), /* EADC1_CH7 */
        .channel = 1,
        .value = 7
    }, {
        .key = PIN(PORT_B, 0), /* EADC1_CH8 */
        .channel = 1,
        .value = 8
    }, {
        .key = PIN(PORT_B, 1), /* EADC1_CH9 */
        .channel = 1,
        .value = 9
    }, {
        .key = PIN(PORT_B, 2), /* EADC1_CH10 */
        .channel = 1,
        .value = 10
    }, {
        .key = PIN(PORT_B, 3), /* EADC1_CH11 */
        .channel = 1,
        .value = 11
    }, {
        .key = PIN(PORT_B, 12), /* EADC1_CH12 */
        .channel = 1,
        .value = 12
    }, {
        .key = PIN(PORT_B, 13), /* EADC1_CH13 */
        .channel = 1,
        .value = 13
    }, {
        .key = PIN(PORT_B, 14), /* EADC1_CH14 */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_B, 15), /* EADC1_CH15 */
        .channel = 1,
        .value = 15
    }, {
        .key = PIN(PORT_C, 13), /* EADC1_CH3 */
        .channel = 1,
        .value = 3
    }, {
        .key = PIN(PORT_D, 10), /* EADC1_CH0 */
        .channel = 1,
        .value = 0
    }, {
        .key = PIN(PORT_D, 11), /* EADC1_CH1 */
        .channel = 1,
        .value = 1
    }, {
        .key = PIN(PORT_D, 12), /* EADC1_CH2 */
        .channel = 1,
        .value = 2
    }, {
        .key = PIN(PORT_C, 9), /* EADC1_ST */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_C, 10), /* EADC1_ST */
        .channel = 1,
        .value = 14
    }, {
        .key = PIN(PORT_F, 4), /* EADC1_ST */
        .channel = 1,
        .value = 11
    }, {
        .key = PIN(PORT_G, 0), /* EADC1_ST */
        .channel = 1,
        .value = 15
    },
#endif
    {
        .key = 0 /* End of pin function association list */
    }
};
/*----------------------------------------------------------------------------*/
static struct EadcBase *instances[2] = {NULL};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_EADC0
void EADC0_P0_ISR(void)
{
  adcBaseHandler0();
}

void EADC0_P1_ISR(void)
{
  adcBaseHandler0();
}

void EADC0_P2_ISR(void)
{
  adcBaseHandler0();
}

void EADC0_P3_ISR(void)
{
  adcBaseHandler0();
}
#endif
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_PLATFORM_NUMICRO_EADC1
void EADC1_P0_ISR(void)
{
  adcBaseHandler1();
}

void EADC1_P1_ISR(void)
{
  adcBaseHandler1();
}

void EADC1_P2_ISR(void)
{
  adcBaseHandler1();
}

void EADC1_P3_ISR(void)
{
  adcBaseHandler1();
}
#endif
/*----------------------------------------------------------------------------*/
void adcBaseHandler0(void)
{
  instances[0]->handler(instances[0]);
}
/*----------------------------------------------------------------------------*/
void adcBaseHandler1(void)
{
  instances[1]->handler(instances[1]);
}
/*----------------------------------------------------------------------------*/
struct EadcBase *adcGetInstance(uint8_t channel)
{
  assert(channel < ARRAY_SIZE(instances));
  return instances[channel];
}
/*----------------------------------------------------------------------------*/
bool adcSetInstance(uint8_t channel, struct EadcBase *expected,
    struct EadcBase *interface)
{
  assert(channel < ARRAY_SIZE(instances));
  return compareExchangePointer(&instances[channel], &expected, interface);
}
/*----------------------------------------------------------------------------*/
static enum Result adcInit(void *object, const void *configBase)
{
  const struct EadcBaseConfig * const config = configBase;
  assert((config->accuracy - 6 <= 6 && config->accuracy % 2 == 0)
      || !config->accuracy);

  struct EadcBase * const interface = object;

  if (!config->shared && !adcSetInstance(config->channel, NULL, interface))
    return E_BUSY;

  enum SysClockBranch clock;
  enum SysBlockReset reset;
  NM_EADC_Type *reg = NULL;
  uint32_t frequency = 0;

  switch (config->channel)
  {
#ifdef CONFIG_PLATFORM_NUMICRO_EADC0
    case 0:
      clock = CLK_EADC0;
      reset = RST_EADC0;
      reg = NM_EADC0;
      frequency = clockFrequency(Eadc0Clock);
      interface->irq.p0 = EADC0_P0_IRQ;
      interface->irq.p1 = EADC0_P1_IRQ;
      interface->irq.p2 = EADC0_P2_IRQ;
      interface->irq.p3 = EADC0_P3_IRQ;
      break;
#endif

#ifdef CONFIG_PLATFORM_NUMICRO_EADC1
    case 1:
      clock = CLK_EADC1;
      reset = RST_EADC1;
      reg = NM_EADC1;
      frequency = clockFrequency(Eadc1Clock);
      interface->irq.p0 = EADC1_P0_IRQ;
      interface->irq.p1 = EADC1_P1_IRQ;
      interface->irq.p2 = EADC1_P2_IRQ;
      interface->irq.p3 = EADC1_P3_IRQ;
      break;
#endif
  }
  assert(frequency > 0 && frequency <= MAX_FREQUENCY);

  uint32_t control = CTL_ADCEN;

  switch (config->accuracy)
  {
    case 6:
      control |= CTL_RESSEL(RESSEL_6BIT);
      break;

    case 8:
      control |= CTL_RESSEL(RESSEL_8BIT);
      break;

    case 10:
      control |= CTL_RESSEL(RESSEL_10BIT);
      break;

    default:
      control |= CTL_RESSEL(RESSEL_12BIT);
      break;
  }

  if (!sysClockStatus(clock))
  {
    /* Enable clock to peripheral */
    sysClockEnable(clock);
    /* Reset registers to default values */
    sysResetBlock(reset);

    /* Reset the peripheral */
    sysUnlockReg();
    reg->CTL = CTL_ADCRST;
    sysLockReg();

    /* Enable deep power-down mode, set LDO start-up time to 20 us */
    reg->PWRM = PWRM_PWUCALEN | PWRM_PWDMOD(PWDMOD_DEEP_POWER_DOWN)
        | PWRM_LDOSUT(frequency / 50000);
    /* Calibrate on each power up */
    reg->CALCTL = CALCTL_CALSEL;
  }

  interface->channel = config->channel;
  interface->control = control;
  interface->handler = NULL;
  interface->reg = reg;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_EADC_NO_DEINIT
static void adcDeinit(void *object)
{
  struct EadcBase * const interface = object;
  adcSetInstance(interface->channel, interface, NULL);
}
#endif
