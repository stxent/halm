/*
 * work_queue.c
 * Copyright (C) 2020 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <uv.h>
#include <halm/generic/work_queue.h>
/*----------------------------------------------------------------------------*/
struct Task
{
  uv_async_t handle;

  void (*callback)(void *);
  void *argument;
};
/*----------------------------------------------------------------------------*/
static void onAsyncCallback(uv_async_t *);
static void onCloseCallback(uv_handle_t *);
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
enum Result workQueueAdd(void (*callback)(void *), void *argument)
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
enum Result workQueueInit(size_t size __attribute__((unused)))
{
  return E_OK;
}
/*----------------------------------------------------------------------------*/
void workQueueStart(void *argument __attribute__((unused)))
{
}
