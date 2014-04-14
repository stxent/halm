/*
 * gppwm.c
 * Copyright (C) 2013 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <assert.h>
#include <platform/nxp/gppwm.h>
#include <platform/nxp/gppwm_defs.h>
/*----------------------------------------------------------------------------*/
#define UNPACK_CHANNEL(value)   (((value) >> 4) & 0x0F)
#define UNPACK_FUNCTION(value)  ((value) & 0x0F)
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcMatchChannel(LPC_PWM_Type *, uint8_t);
static int8_t setupMatchPin(uint8_t channel, gpio_t key);
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *, const void *);
static void unitDeinit(void *);
/*----------------------------------------------------------------------------*/
static uint32_t channelGetResolution(void *);
static void channelSetEnabled(void *, bool);
/*----------------------------------------------------------------------------*/
static enum result singleEdgeInit(void *, const void *);
static void singleEdgeDeinit(void *);
static void singleEdgeSetDuration(void *, uint32_t);
static void singleEdgeSetEdges(void *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static enum result doubleEdgeInit(void *, const void *);
static void doubleEdgeDeinit(void *);
static void doubleEdgeSetDuration(void *, uint32_t);
static void doubleEdgeSetEdges(void *, uint32_t, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct EntityClass unitTable = {
    .size = sizeof(struct GpPwmUnit),
    .init = unitInit,
    .deinit = unitDeinit
};
/*----------------------------------------------------------------------------*/
static const struct PwmClass singleEdgeTable = {
    .size = sizeof(struct GpPwm),
    .init = singleEdgeInit,
    .deinit = singleEdgeDeinit,

    .getResolution = channelGetResolution,
    .setDuration = singleEdgeSetDuration,
    .setEdges = singleEdgeSetEdges,
    .setEnabled = channelSetEnabled
};
/*----------------------------------------------------------------------------*/
static const struct PwmClass doubleEdgeTable = {
    .size = sizeof(struct GpPwmDoubleEdge),
    .init = doubleEdgeInit,
    .deinit = doubleEdgeDeinit,

    .getResolution = channelGetResolution,
    .setDuration = doubleEdgeSetDuration,
    .setEdges = doubleEdgeSetEdges,
    .setEnabled = channelSetEnabled
};
/*----------------------------------------------------------------------------*/
extern const struct GpioDescriptor gpPwmPins[];
const struct EntityClass *GpPwmUnit = &unitTable;
const struct PwmClass *GpPwm = &singleEdgeTable;
const struct PwmClass *GpPwmDoubleEdge = &doubleEdgeTable;
/*----------------------------------------------------------------------------*/
static inline uint32_t *calcMatchChannel(LPC_PWM_Type *device, uint8_t channel)
{
  assert(channel && channel <= 6);

  if (channel > 3)
    return (uint32_t *)&device->MR4 + (channel - 4);
  else
    return (uint32_t *)&device->MR1 + (channel - 1);
}
/*----------------------------------------------------------------------------*/
static int8_t setupMatchPin(uint8_t channel, gpio_t key)
{
  const struct GpioDescriptor *pinDescriptor;

  if (!(pinDescriptor = gpioFind(gpPwmPins, key, channel)))
    return -1;

  struct Gpio pin = gpioInit(key);
  gpioOutput(pin, 0);
  gpioSetFunction(pin, UNPACK_FUNCTION(pinDescriptor->value));

  return UNPACK_CHANNEL(pinDescriptor->value);
}
/*----------------------------------------------------------------------------*/
static enum result unitInit(void *object, const void *configPtr)
{
  const struct GpPwmUnitConfig * const config = configPtr;
  const struct GpPwmUnitBaseConfig parentConfig = {
      .channel = config->channel
  };
  struct GpPwmUnit *unit = object;
  enum result res;

  const uint32_t clockFrequency = gpPwmGetClock(object);
  const uint32_t timerFrequency = config->frequency * config->resolution;

  assert(timerFrequency && timerFrequency < clockFrequency);

  /* Call base class constructor */
  if ((res = GpPwmUnitBase->init(object, &parentConfig)) != E_OK)
    return res;

  unit->resolution = config->resolution;
  unit->matches = 0;

  LPC_PWM_Type *reg = unit->parent.reg;

  reg->TCR = 0;

  reg->IR = reg->IR; /* Clear pending interrupts */
  reg->PC = reg->TC = 0;
  reg->CTCR = 0;
  reg->CCR = 0;
  reg->PCR = 0;

  /* Configure timings */
  reg->PR = clockFrequency / timerFrequency - 1;
  reg->MR0 = unit->resolution;
  reg->MCR = MCR_RESET(0);

  /* Enable timer and PWM mode */
  reg->TCR = TCR_CEN | TCR_PWM_ENABLE;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void unitDeinit(void *object)
{
  struct GpPwmUnit *unit = object;

  ((LPC_PWM_Type *)unit->parent.reg)->TCR &= ~(TCR_CEN | TCR_PWM_ENABLE);
  GpPwmUnitBase->deinit(object);
}
/*----------------------------------------------------------------------------*/
static uint32_t channelGetResolution(void *object)
{
  return ((struct GpPwm *)object)->unit->resolution;
}
/*----------------------------------------------------------------------------*/
static void channelSetEnabled(void *object, bool state)
{
  struct GpPwm *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  if (!state)
    reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
  else
    reg->PCR |= PCR_OUTPUT_ENABLED(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static enum result singleEdgeInit(void *object, const void *configPtr)
{
  const struct GpPwmConfig * const config = configPtr;
  struct GpPwm *pwm = object;
  int8_t channel;

  /* Initialize output pin */
  channel = setupMatchPin(config->parent->parent.channel, config->pin);
  if (channel == -1)
    return E_VALUE;

  /* Check whether channel is free */
  if (config->parent->matches & (1 << channel))
    return E_BUSY;

  pwm->channel = (uint8_t)channel;
  pwm->unit = config->parent;
  pwm->unit->matches |= 1 << channel;

  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  /* Calculate pointer to match register for fast access */
  pwm->value = calcMatchChannel(reg, channel);
  /* Call function directly because of unfinished object construction */
  singleEdgeSetDuration(pwm, config->duration);
  /* Enable channel */
  reg->PCR |= PCR_OUTPUT_ENABLED(channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void singleEdgeDeinit(void *object)
{
  struct GpPwm *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
  pwm->unit->matches &= ~(1 << pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetDuration(void *object, uint32_t duration)
{
  struct GpPwm *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  if (duration >= pwm->unit->resolution)
  {
    /*
     * If match register is set to a value greater than resolution,
     * than output stays high during all cycle.
     */
    duration = pwm->unit->resolution + 1;
  }

  *pwm->value = duration;
  reg->LER |= LER_ENABLE_LATCH(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static void singleEdgeSetEdges(void *object,
    uint32_t leading __attribute__((unused)), uint32_t trailing)
{
  struct GpPwm *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  assert(!leading); /* Leading edge time is constant in single edge mode */

  if (trailing >= pwm->unit->resolution)
    trailing = pwm->unit->resolution + 1;

  *pwm->value = trailing;
  reg->LER |= LER_ENABLE_LATCH(pwm->channel);
}
/*----------------------------------------------------------------------------*/
static enum result doubleEdgeInit(void *object, const void *configPtr)
{
  const struct GpPwmDoubleEdgeConfig * const config = configPtr;
  struct GpPwmDoubleEdge *pwm = object;
  int8_t channel;

  /* Initialize output pin */
  channel = setupMatchPin(config->parent->parent.channel, config->pin);
  /* First channel cannot be a double edged output */
  if (channel <= 1)
    return E_VALUE;

  /* Check whether channels are free */
  if (config->parent->matches & (1 << channel | 1 << (channel - 1)))
    return E_BUSY;

  pwm->channel = (uint8_t)channel;
  pwm->unit = config->parent;
  pwm->unit->matches |= 1 << channel | 1 << (channel - 1);

  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  /* Setup channels and initial edge times */
  pwm->leading = calcMatchChannel(reg, channel - 1);
  pwm->trailing = calcMatchChannel(reg, channel);
  doubleEdgeSetEdges(pwm, config->leading, config->trailing);
  /* Select double edge mode and enable the channel */
  reg->PCR |= PCR_DOUBLE_EDGE(channel) | PCR_OUTPUT_ENABLED(channel);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeDeinit(void *object)
{
  struct GpPwmDoubleEdge *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  reg->PCR &= ~PCR_OUTPUT_ENABLED(pwm->channel);
  pwm->unit->matches &= ~(1 << pwm->channel | 1 << (pwm->channel - 1));
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetDuration(void *object, uint32_t duration)
{
  struct GpPwmDoubleEdge *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;
  uint32_t center, leading, resolution, trailing;

  leading = *pwm->leading;
  trailing = *pwm->trailing;
  resolution = pwm->unit->resolution;

  if (leading > trailing)
  {
    uint32_t half = resolution / 2;

    center = trailing + (leading - trailing) / 2;
    if (center < half)
      center += half;
    else
      center -= half;
  }
  else
  {
    center = leading;
    if (trailing < resolution)
      center += (trailing - leading) / 2;
  }

  if (duration >= resolution)
  {
    leading = center;
    trailing = resolution + 1;
  }
  else
  {
    duration = duration / 2;

    leading = center >= duration ? center - duration
        : center - duration + resolution;
    trailing = center <= duration ? center + duration
        : center + duration - resolution;
  }

  *pwm->leading = leading;
  *pwm->trailing = trailing;
  reg->LER |= LER_ENABLE_LATCH(pwm->channel)
      | LER_ENABLE_LATCH(pwm->channel - 1);
}
/*----------------------------------------------------------------------------*/
static void doubleEdgeSetEdges(void *object, uint32_t leading,
    uint32_t trailing)
{
  struct GpPwmDoubleEdge *pwm = object;
  LPC_PWM_Type *reg = pwm->unit->parent.reg;

  assert(leading < pwm->unit->resolution);

  if (trailing >= pwm->unit->resolution)
    trailing = pwm->unit->resolution + 1;

  *pwm->leading = leading;
  *pwm->trailing = trailing;
  reg->LER |= LER_ENABLE_LATCH(pwm->channel)
      | LER_ENABLE_LATCH(pwm->channel - 1);
}
/*----------------------------------------------------------------------------*/
/**
 * Create single edge PWM channel.
 * @param unit Pointer to a GpPwmUnit object.
 * @param pin Pin used as output for pulse width modulated signal.
 * @param duration Initial duration in timer ticks.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *gpPwmCreate(void *unit, gpio_t pin, uint32_t duration)
{
  const struct GpPwmConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .duration = duration
  };

  return init(GpPwm, &channelConfig);
}
/*----------------------------------------------------------------------------*/
/**
 * Create double edge PWM channel.
 * @param unit Pointer to a GpPwmUnit object.
 * @param pin Pin used as output for pulse width modulated signal.
 * @param duration Initial duration in timer ticks.
 * @return Pointer to a new Pwm object on success or zero on error.
 */
void *gpPwmCreateDoubleEdge(void *unit, gpio_t pin, uint32_t leading,
    uint32_t trailing)
{
  const struct GpPwmDoubleEdgeConfig channelConfig = {
      .parent = unit,
      .pin = pin,
      .leading = leading,
      .trailing = trailing
  };

  return init(GpPwmDoubleEdge, &channelConfig);
}
