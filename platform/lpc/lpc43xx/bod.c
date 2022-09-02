/*
 * bod.c
 * Copyright (C) 2022 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/lpc/bod.h>
#include <halm/platform/lpc/lpc43xx/event_router.h>
#include <halm/platform/lpc/lpc43xx/system_defs.h>
#include <halm/platform/platform_defs.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/
static void interruptHandler(void *);
/*----------------------------------------------------------------------------*/
static enum Result bodInit(void *, const void *);
static void bodEnable(void *);
static void bodDisable(void *);
static void bodSetCallback(void *, void (*)(void *), void *);

#ifndef CONFIG_PLATFORM_NXP_BOD_NO_DEINIT
static void bodDeinit(void *);
#else
#define bodDeinit deletedDestructorTrap
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
static void interruptHandler(void *object)
{
  struct Bod * const bod = object;

  if (bod->enabled && bod->callback)
    bod->callback(bod->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result bodInit(void *object, const void *configBase)
{
  const struct BodConfig * const config = configBase;
  assert(config);
  assert(config->eventLevel <= BOD_EVENT_3V05);
  assert(config->resetLevel <= BOD_RESET_2V2);

  struct Bod * const bod = object;
  uint32_t creg = LPC_CREG->CREG0 & ~(CREG0_BODLVL1_MASK | CREG0_BODLVL2_MASK);

  bod->callback = 0;
  bod->enabled = false;

  creg |= CREG0_BODLVL1(config->eventLevel);
  creg |= CREG0_BODLVL2(config->resetLevel);

  LPC_CREG->CREG0 = creg;

  return erRegister(interruptHandler, bod, ER_BOD);
}
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_PLATFORM_NXP_BOD_NO_DEINIT
static void bodDeinit(void *object)
{
  erUnregister(object);
}
#endif
/*----------------------------------------------------------------------------*/
static void bodEnable(void *object)
{
  struct Bod * const bod = object;
  bod->enabled = true;
}
/*----------------------------------------------------------------------------*/
static void bodDisable(void *object)
{
  struct Bod * const bod = object;
  bod->enabled = false;
}
/*----------------------------------------------------------------------------*/
static void bodSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Bod * const bod = object;

  bod->callbackArgument = argument;
  bod->callback = callback;
}
