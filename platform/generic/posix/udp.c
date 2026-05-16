/*
 * udp.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/udp.h>
#include <xcore/containers/byte_queue.h>
#include <uv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_SIZE 1536
#define QUEUE_SIZE  2048
/*----------------------------------------------------------------------------*/
enum Cleanup
{
  CLEANUP_ALL,
  CLEANUP_NETWORK,
  CLEANUP_LISTENER,
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

  uv_poll_t *listener;
  int server;

  struct sockaddr_in client;
  bool clientConfigured;
};
/*----------------------------------------------------------------------------*/
static void cleanup(struct Udp *, enum Cleanup);
static void onCloseCallback(uv_handle_t *);
static void onInterfaceCallback(uv_poll_t *, int, int);
static enum Result setupSockets(struct Udp *, const struct UdpConfig *);
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *, const void *);
static void streamDeinit(void *);
static void streamSetCallback(void *, void (*)(void *), void *);
static enum Result streamGetParam(void *, int, void *);
static enum Result streamSetParam(void *, int, const void *);
static size_t streamRead(void *, void *, size_t);
static size_t streamWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Udp = &(const struct InterfaceClass){
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
static void cleanup(struct Udp *interface, enum Cleanup step)
{
  switch (step)
  {
    case CLEANUP_ALL:
      uv_handle_set_data((uv_handle_t *)interface->listener, NULL);
      uv_close((uv_handle_t *)interface->listener, onCloseCallback);
      [[fallthrough]];
    case CLEANUP_NETWORK:
      close(interface->server);
      [[fallthrough]];
    case CLEANUP_QUEUE:
      byteQueueDeinit(&interface->rxQueue);
      [[fallthrough]];
    case CLEANUP_LISTENER:
      if (step != CLEANUP_ALL)
        free(interface->listener);
      [[fallthrough]];
    case CLEANUP_MUTEX:
      pthread_mutex_destroy(&interface->rxQueueLock);
      break;
  }
}
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *handle)
{
  free(handle);
}
/*----------------------------------------------------------------------------*/
static void onInterfaceCallback(uv_poll_t *handle, int, int)
{
  struct Udp * const interface = uv_handle_get_data((uv_handle_t *)handle);
  uint8_t buffer[BUFFER_SIZE];
  ssize_t length;

  pthread_mutex_lock(&interface->rxQueueLock);
  do
  {
    struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);

    length = recvfrom(interface->server, buffer, sizeof(buffer), 0,
        (struct sockaddr *)&address, &addressLength);

    if (!interface->clientConfigured && addressLength)
    {
      /* Update client address */
      interface->client = address;
    }

    if (length > 0)
      byteQueuePushArray(&interface->rxQueue, buffer, (size_t)length);
  }
  while (length > 0);
  pthread_mutex_unlock(&interface->rxQueueLock);

  if (interface->callback != NULL)
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

  /* Initialize output address */
  memset(&address, 0, sizeof(address));
  if (config->clientAddress != NULL && config->clientPort)
  {
    if (inet_pton(AF_INET, config->clientAddress, &address.sin_addr) <= 0)
      return E_VALUE;

    address.sin_family = AF_INET;
    address.sin_port = htons(config->clientPort);
    interface->clientConfigured = true;
  }
  else
  {
    if (config->clientAddress != NULL || config->clientPort)
      return E_VALUE;

    address.sin_family = AF_UNSPEC;
    interface->clientConfigured = false;
  }
  interface->client = address;

  /* Initialize input socket */
  interface->server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (interface->server == -1)
    return E_INTERFACE;

  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(config->serverPort);
  if (config->serverAddress != NULL)
  {
    if (inet_pton(AF_INET, config->serverAddress, &address.sin_addr) <= 0)
    {
      res = E_VALUE;
      goto close_server;
    }
  }
  else
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
  return res;
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct UdpConfig * const config = configBase;
  struct Udp * const interface = object;
  enum Result res;

  interface->callback = NULL;

  if (pthread_mutex_init(&interface->rxQueueLock, 0))
    return E_ERROR;

  interface->listener = malloc(sizeof(uv_poll_t));
  if (interface->listener == NULL)
  {
    cleanup(interface, CLEANUP_MUTEX);
    return E_MEMORY;
  }

  if (!byteQueueInit(&interface->rxQueue, QUEUE_SIZE))
  {
    cleanup(interface, CLEANUP_LISTENER);
    return E_MEMORY;
  }

  if ((res = setupSockets(interface, config)) != E_OK)
  {
    cleanup(interface, CLEANUP_QUEUE);
    return res;
  }

  /* Initialize and start thread */
  uv_poll_init_socket(uv_default_loop(), interface->listener,
      interface->server);
  uv_handle_set_data((uv_handle_t *)interface->listener, interface);
  uv_poll_start(interface->listener, UV_READABLE, onInterfaceCallback);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object)
{
  cleanup(object, CLEANUP_ALL);
}
/*----------------------------------------------------------------------------*/
static void streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Udp * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result streamGetParam(void *object, int parameter, void *data)
{
  struct Udp *interface = object;

  switch ((enum IfParameter)parameter)
  {
    case IF_RX_AVAILABLE:
      pthread_mutex_lock(&interface->rxQueueLock);
      *(size_t *)data = byteQueueSize(&interface->rxQueue);
      pthread_mutex_unlock(&interface->rxQueueLock);
      return E_OK;

    case IF_RX_PENDING:
      pthread_mutex_lock(&interface->rxQueueLock);
      *(size_t *)data = byteQueueCapacity(&interface->rxQueue)
          - byteQueueSize(&interface->rxQueue);
      pthread_mutex_unlock(&interface->rxQueueLock);
      return E_OK;

    case IF_TX_AVAILABLE:
      *(size_t *)data = 0;
      return E_OK;

    case IF_TX_PENDING:
      *(size_t *)data = 0;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamSetParam(void *, int, const void *)
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

  if (interface->client.sin_family == AF_INET)
  {
    const ssize_t written = sendto(interface->server, buffer, length, 0,
        (const struct sockaddr *)&interface->client,
        sizeof(struct sockaddr_in));

    return written > 0 ? (size_t)written : 0;
  }
  else
    return 0;
}
