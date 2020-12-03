/*
 * console.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/console.h>
#include <xcore/containers/byte_queue.h>
#include <uv.h>
#include <pthread.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_SIZE 64
#define QUEUE_SIZE  2048
/*----------------------------------------------------------------------------*/
enum Cleanup
{
  CLEANUP_ALL,
  CLEANUP_THREAD,
  CLEANUP_MUTEX,
  CLEANUP_SEMAPHORE,
  CLEANUP_QUEUE
};

struct Console
{
  struct Interface base;

  void (*callback)(void *);
  void *callbackArgument;

  struct ByteQueue rxQueue;
  pthread_mutex_t rxQueueLock;

  struct termios initialSettings;
  uv_poll_t *listener;
};
/*----------------------------------------------------------------------------*/
static void configurePort(struct Console *);
static void onCloseCallback(uv_handle_t *);
static void onInterfaceCallback(uv_poll_t *, int, int);
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *, const void *);
static void streamDeinit(void *);
static void streamSetCallback(void *, void (*)(void *), void *);
static enum Result streamGetParam(void *, int, void *);
static enum Result streamSetParam(void *, int, const void *);
static size_t streamRead(void *, void *, size_t);
static size_t streamWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Console = &(const struct InterfaceClass){
    .size = sizeof(struct Console),
    .init = streamInit,
    .deinit = streamDeinit,

    .setCallback = streamSetCallback,
    .getParam = streamGetParam,
    .setParam = streamSetParam,
    .read = streamRead,
    .write = streamWrite
};
/*----------------------------------------------------------------------------*/
static void configurePort(struct Console *interface)
{
  struct termios settings;

  tcgetattr(STDIN_FILENO, &settings);
  interface->initialSettings = settings;

  settings.c_lflag &= ~(ISIG | ICANON | ECHO);
  settings.c_cc[VMIN] = 0; /* Minimal data packet length */
  settings.c_cc[VTIME] = 0; /* Time to wait for data */

  tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *handle)
{
  free(handle);
}
/*----------------------------------------------------------------------------*/
static void onInterfaceCallback(uv_poll_t *handle,
    int status __attribute__((unused)),
    int events __attribute__((unused)))
{
  struct Console * const interface = handle->data;
  uint8_t buffer[BUFFER_SIZE];
  ssize_t length;

  pthread_mutex_lock(&interface->rxQueueLock);
  while ((length = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0)
    byteQueuePushArray(&interface->rxQueue, buffer, (size_t)length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object,
    const void *configBase __attribute__((unused)))
{
  struct Console * const interface = object;
  enum Result res;

  interface->callback = 0;

  if (pthread_mutex_init(&interface->rxQueueLock, 0))
    return E_ERROR;

  interface->listener = malloc(sizeof(uv_poll_t));
  if (!interface->listener)
  {
    res = E_MEMORY;
    goto free_mutex;
  }

  if (!byteQueueInit(&interface->rxQueue, QUEUE_SIZE))
  {
    res = E_MEMORY;
    goto free_listener;
  }

  configurePort(interface);

  uv_poll_init(uv_default_loop(), interface->listener, STDIN_FILENO);
  uv_handle_set_data((uv_handle_t *)interface->listener, interface);
  uv_poll_start(interface->listener, UV_READABLE, onInterfaceCallback);

  return E_OK;

free_listener:
  free(interface->listener);
free_mutex:
  pthread_mutex_destroy(&interface->rxQueueLock);
  return res;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object __attribute__((unused)))
{
  struct Console * const interface = object;

  uv_handle_set_data((uv_handle_t *)interface->listener, 0);
  uv_close((uv_handle_t *)interface->listener, onCloseCallback);

  /* Restore terminal settings */
  tcsetattr(STDIN_FILENO, TCSANOW, &interface->initialSettings);

  byteQueueDeinit(&interface->rxQueue);
  pthread_mutex_destroy(&interface->rxQueueLock);
}
/*----------------------------------------------------------------------------*/
static void streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Console * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result streamGetParam(void *object, int parameter, void *data)
{
  struct Console *interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_AVAILABLE:
      pthread_mutex_lock(&interface->rxQueueLock);
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      pthread_mutex_unlock(&interface->rxQueueLock);
      return E_OK;

    case IF_PENDING:
      *(size_t *)data = 0;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamSetParam(void *object __attribute__((unused)),
    int parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t streamRead(void *object, void *buffer, size_t length)
{
  struct Console * const interface = object;

  pthread_mutex_lock(&interface->rxQueueLock);
  const size_t read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t streamWrite(void *object __attribute__((unused)),
    const void *buffer, size_t length)
{
  if (!length)
    return 0;

  const ssize_t written = write(STDOUT_FILENO, buffer, length);

  if (written)
  {
    fsync(STDOUT_FILENO);
    return (size_t)written;
  }
  else
  {
    return 0;
  }
}
