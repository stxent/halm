/*
 * signal_handler.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/signal_handler.h>
#include <uv.h>
#include <assert.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
struct SignalHandler
{
  struct Interrupt base;

  void (*callback)(void *);
  void *callbackArgument;

  uv_signal_t *handle;
  int signum;
};
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *);
static void onSignalReceived(uv_signal_t *, int);
/*----------------------------------------------------------------------------*/
static enum Result shInit(void *, const void *);
static void shDeinit(void *);
static void shEnable(void *);
static void shDisable(void *);
static void shSetCallback(void *, void (*)(void *), void *);
/*----------------------------------------------------------------------------*/
const struct InterruptClass * const SignalHandler =
    &(const struct InterruptClass){
    .size = sizeof(struct SignalHandler),
    .init = shInit,
    .deinit = shDeinit,

    .enable = shEnable,
    .disable = shDisable,
    .setCallback = shSetCallback
};
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *handle)
{
  free(handle);
}
/*----------------------------------------------------------------------------*/
static void onSignalReceived(uv_signal_t *handle, int)
{
  struct SignalHandler * const handler =
      uv_handle_get_data((uv_handle_t *)handle);

  if (handler->callback != NULL)
    handler->callback(handler->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result shInit(void *object, const void *configBase)
{
  const struct SignalHandlerConfig * const config = configBase;
  assert(config != NULL);

  struct SignalHandler * const handler = object;

  handler->handle = malloc(sizeof(uv_signal_t));
  if (handler->handle == NULL)
    return E_MEMORY;

  if (uv_signal_init(uv_default_loop(), handler->handle) < 0)
  {
    free(handler->handle);
    return E_ERROR;
  }

  handler->callback = NULL;
  handler->signum = config->signum;
  uv_handle_set_data((uv_handle_t *)handler->handle, handler);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void shDeinit(void *object)
{
  struct SignalHandler * const handler = object;

  uv_handle_set_data((uv_handle_t *)handler->handle, NULL);
  uv_close((uv_handle_t *)handler->handle, onCloseCallback);
}
/*----------------------------------------------------------------------------*/
static void shEnable(void *object)
{
  struct SignalHandler * const handler = object;
  uv_signal_start(handler->handle, onSignalReceived, handler->signum);
}
/*----------------------------------------------------------------------------*/
static void shDisable(void *object)
{
  struct SignalHandler * const handler = object;
  uv_signal_stop(handler->handle);
}
/*----------------------------------------------------------------------------*/
static void shSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct SignalHandler * const handler = object;

  handler->callbackArgument = argument;
  handler->callback = callback;
}
