/*
 * event_queue.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/event_queue.h>
#include <uv.h>
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
struct Task
{
  uv_async_t handle;

  void (*callback)(void *);
  void *argument;
};

struct EventQueue
{
  struct WorkQueue base;
};
/*----------------------------------------------------------------------------*/
static void onAsyncCallback(uv_async_t *);
static void onCloseCallback(uv_handle_t *);

static enum Result workQueueInit(void *, const void *);
static enum Result workQueueAdd(void *, void (*)(void *), void *);
static enum Result workQueueStart(void *);
static void workQueueStop(void *);
/*----------------------------------------------------------------------------*/
const struct WorkQueueClass * const EventQueue =
    &(const struct WorkQueueClass){
    .size = sizeof(struct EventQueue),
    .init = workQueueInit,
    .deinit = deletedDestructorTrap,

    .add = workQueueAdd,
    .start = workQueueStart,
    .stop = workQueueStop
};
/*----------------------------------------------------------------------------*/
static void onAsyncCallback(uv_async_t *handle)
{
  struct Task * const task = uv_handle_get_data((uv_handle_t *)handle);

  task->callback(task->argument);
  uv_close((uv_handle_t *)handle, onCloseCallback);
}
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *handle)
{
  free(uv_handle_get_data((uv_handle_t *)handle));
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueInit(void *object __attribute__((unused)),
    const void *configBase __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueAdd(void *object __attribute__((unused)),
    void (*callback)(void *), void *argument)
{
  if (!callback)
    return E_VALUE;

  struct Task * const task = malloc(sizeof(struct Task));
  if (!task)
    return E_MEMORY;

  if (uv_async_init(uv_default_loop(), &task->handle, onAsyncCallback) < 0)
  {
    free(task);
    return E_ERROR;
  }

  uv_handle_set_data((uv_handle_t *)&task->handle, task);
  task->callback = callback;
  task->argument = argument;

  if (uv_async_send(&task->handle) < 0)
  {
    free(task);
    return E_ERROR;
  }

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result workQueueStart(void *object __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void workQueueStop(void *object __attribute__((unused)))
{
}
