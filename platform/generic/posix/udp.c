/*
 * udp.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <uv.h>
#include <xcore/containers/byte_queue.h>
#include <halm/platform/generic/udp.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_SIZE 1536
#define QUEUE_SIZE  2048
/*----------------------------------------------------------------------------*/
enum Cleanup
{
  CLEANUP_ALL,
  CLEANUP_NETWORK,
  CLEANUP_MUTEX,
  CLEANUP_QUEUE
};

struct Udp
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  struct ByteQueue rxQueue;
  pthread_mutex_t rxQueueLock;

  uv_poll_t listener;
  int client;
  int server;
};
/*----------------------------------------------------------------------------*/
static void cleanup(struct Udp *, enum Cleanup);
static void interfaceCallback(uv_poll_t *, int, int);
static enum Result setupSockets(struct Udp *,
    const struct UdpConfig *);
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *, const void *);
static void streamDeinit(void *);
static enum Result streamSetCallback(void *, void (*)(void *), void *);
static enum Result streamGetParam(void *, enum IfParameter, void *);
static enum Result streamSetParam(void *, enum IfParameter, const void *);
static size_t streamRead(void *, void *, size_t);
static size_t streamWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass streamTable = {
    .size = sizeof(struct Udp),
    .init = streamInit,
    .deinit = streamDeinit,

    .setCallback = streamSetCallback,
    .getParam = streamGetParam,
    .setParam = streamSetParam,
    .read = streamRead,
    .write = streamWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Udp = &streamTable;
/*----------------------------------------------------------------------------*/
static void cleanup(struct Udp *interface, enum Cleanup step)
{
  switch (step)
  {
    case CLEANUP_ALL:
      uv_poll_stop(&interface->listener);
      /* Falls through */
    case CLEANUP_NETWORK:
      close(interface->server);
      close(interface->client);
      /* Falls through */
    case CLEANUP_QUEUE:
      byteQueueDeinit(&interface->rxQueue);
      /* Falls through */
    case CLEANUP_MUTEX:
      pthread_mutex_destroy(&interface->rxQueueLock);
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void interfaceCallback(uv_poll_t *handle,
    int status __attribute__((unused)),
    int events __attribute__((unused)))
{
  struct Udp * const interface = handle->data;
  uint8_t buffer[BUFFER_SIZE];
  ssize_t length;

  pthread_mutex_lock(&interface->rxQueueLock);
  while ((length = recv(interface->server, buffer, sizeof(buffer), 0)) > 0)
    byteQueuePushArray(&interface->rxQueue, buffer, (size_t)length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static enum Result setupSockets(struct Udp *interface,
    const struct UdpConfig *config)
{
  const struct timeval timeout = {
      .tv_sec = 0,
      .tv_usec = 100
  };
  struct sockaddr_in address;
  enum Result res = E_OK;

  /* Initialize output socket */
  interface->client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (interface->client == -1)
    return E_INTERFACE;

  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(config->clientPort);
  if (inet_pton(AF_INET, config->clientAddress, &address.sin_addr) <= 0)
  {
    res = E_VALUE;
    goto close_client;
  }
  if (connect(interface->client, (struct sockaddr *)&address,
      sizeof(address)) == -1)
  {
    res = E_BUSY;
    goto close_client;
  }

  /* Initialize input socket */
  interface->server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (interface->server == -1)
  {
    res = E_INTERFACE;
    goto close_client;
  }

  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(config->serverPort);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(interface->server, (struct sockaddr *)&address,
      sizeof(address)) == -1)
  {
    res = E_BUSY;
    goto close_server;
  }

  if (setsockopt(interface->server, SOL_SOCKET, SO_RCVTIMEO,
      &timeout, sizeof(struct timeval)) == -1)
  {
    res = E_INTERFACE;
    goto close_server;
  }

  return E_OK;

close_server:
  close(interface->server);
close_client:
  close(interface->client);
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct UdpConfig * const config = configBase;
  struct Udp * const interface = object;
  enum Result res;

  interface->callback = 0;

  if (pthread_mutex_init(&interface->rxQueueLock, 0))
    return E_ERROR;

  if ((res = byteQueueInit(&interface->rxQueue, QUEUE_SIZE)) != E_OK)
  {
    cleanup(interface, CLEANUP_MUTEX);
    return res;
  }

  if ((res = setupSockets(interface, config)) != E_OK)
  {
    cleanup(interface, CLEANUP_QUEUE);
    return res;
  }

  /* Initialize and start thread */
  uv_poll_init_socket(uv_default_loop(), &interface->listener,
      interface->server);
  interface->listener.data = interface;
  uv_poll_start(&interface->listener, UV_READABLE, interfaceCallback);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object)
{
  cleanup(object, CLEANUP_ALL);
}
/*----------------------------------------------------------------------------*/
static enum Result streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Udp * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result streamGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct Udp *interface = object;

  switch (parameter)
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
    enum IfParameter parameter __attribute__((unused)),
    const void *data __attribute__((unused)))
{
  return E_INVALID;
}
/*----------------------------------------------------------------------------*/
static size_t streamRead(void *object, void *buffer, size_t length)
{
  struct Udp * const interface = object;

  pthread_mutex_lock(&interface->rxQueueLock);
  const size_t read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t streamWrite(void *object, const void *buffer, size_t length)
{
  struct Udp * const interface = object;
  const int written = send(interface->client, buffer, length, 0);

  return written > 0 ? (size_t)written : 0;
}
