/*
 * bod.c
 * Copyright (C) 2026 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/numicro/bod.h>
#include <halm/platform/numicro/system.h>
#include <halm/platform/numicro/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Bod *);
/*----------------------------------------------------------------------------*/
static enum Result bodInit(void *, const void *);
static void bodEnable(void *);
static void bodDisable(void *);
static void bodSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_NUMICRO_BOD_NO_DEINIT
static void bodDeinit(void *);
#else
#  define bodDeinit deletedDestructorTrap
#endif
/*----------------------------------------------------------------------------*/
const struct InterruptClass * const Bod = &(const struct InterruptClass){
    .size = sizeof(struct Bod),
    .init = bodInit,
    .deinit = bodDeinit,

    .enable = bodEnable,
    .disable = bodDisable,
    .setCallback = bodSetCallback
};
/*----------------------------------------------------------------------------*/
static struct Bod *instance = NULL;
/*----------------------------------------------------------------------------*/
static bool setInstance(struct Bod *object)
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
void BOD_ISR(void)
{
  if (instance->callback != NULL)
  {
    const bool high = (NM_SYS->BODCTL & BODCTL_BODOUT) == 0;
    const enum InputEvent event = instance->event;
    bool invoke = event == INPUT_TOGGLE;

    /* Clear interrupt flag, flag is not write-protected */
    NM_SYS->BODCTL |= BODCTL_BODIF;

    if ((high && event == INPUT_RISING) || (!high && event == INPUT_FALLING))
      invoke = true;

    if (invoke)
      instance->callback(instance->callbackArgument);
  }
}
/*----------------------------------------------------------------------------*/
static enum Result bodInit(void *object, const void *configBase)
{
  const struct BodConfig * const config = configBase;
  assert(config != NULL);
  assert(config->event != INPUT_HIGH && config->event != INPUT_LOW);
  assert(config->level < BOD_LEVEL_END);
  assert(config->timeout < BOD_TIMEOUT_END);

  struct Bod * const bod = object;

  if (!setInstance(bod))
    return E_BUSY;

  uint32_t bodctl = NM_SYS->BODCTL
      & ~(BODCTL_BODRSTEN | BODCTL_BODDGSEL_MASK | BODCTL_BODVL_MASK);

  bod->callback = NULL;
  bod->event = config->event;

  bodctl |= BODCTL_BODEN;
  if (config->reset)
    bodctl |= BODCTL_BODRSTEN;
  if (config->slow)
    bodctl |= BODCTL_BODLPM;
  bodctl |= BODCTL_BODDGSEL(config->timeout) | BODCTL_BODVL(config->level);

  sysUnlockReg();
  NM_SYS->BODCTL = bodctl;
  sysLockReg();

  irqSetPriority(BOD_IRQ, config->priority);
  return E_OK;
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NUMICRO_BOD_NO_DEINIT
static void bodDeinit(void *)
{
  irqDisable(BOD_IRQ);

  sysUnlockReg();
  NM_SYS->BODCTL &= ~(BODCTL_BODEN | BODCTL_BODRSTEN);
  sysLockReg();

  instance = NULL;
}
#endif
/*----------------------------------------------------------------------------*/
static void bodEnable(void *)
{
  NM_SYS->BODCTL |= BODCTL_BODIF;
  irqEnable(BOD_IRQ);
}
/*----------------------------------------------------------------------------*/
static void bodDisable(void *)
{
  irqDisable(BOD_IRQ);
}
/*----------------------------------------------------------------------------*/
static void bodSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Bod * const bod = object;

  bod->callbackArgument = argument;
  bod->callback = callback;
}
