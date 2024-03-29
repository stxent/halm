/*
 * serial.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/platform/generic/serial.h>
#include <xcore/bits.h>
#include <xcore/containers/byte_queue.h>
#include <uv.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_SIZE 64
#define QUEUE_SIZE  2048
/*----------------------------------------------------------------------------*/
struct StreamRateEntry
{
  uint32_t key;
  speed_t value;
};

struct Serial
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  struct ByteQueue rxQueue;
  pthread_mutex_t rxQueueLock;

  struct termios initialSettings;
  uv_poll_t *listener;
  int descriptor;
};
/*----------------------------------------------------------------------------*/
static const struct StreamRateEntry rateList[] = {
    {.key = 1200,    .value = B1200},
    {.key = 2400,    .value = B2400},
    {.key = 4800,    .value = B4800},
    {.key = 9600,    .value = B9600},
    {.key = 19200,   .value = B19200},
    {.key = 38400,   .value = B38400},
    {.key = 57600,   .value = B57600},
    {.key = 115200,  .value = B115200},
    {.key = 230400,  .value = B230400},
    {.key = 460800,  .value = B460800},
    {.key = 500000,  .value = B500000},
    {.key = 576000,  .value = B576000},
    {.key = 921600,  .value = B921600},
    {.key = 1000000, .value = B1000000}
};
/*----------------------------------------------------------------------------*/
static bool changePortFlag(struct Serial *, int, uint8_t);
static bool getPortFlag(struct Serial *, int, uint8_t *);
static bool getPortParity(struct Serial *, uint8_t *);
static bool getPortRate(struct Serial *, uint32_t *);
static void onCloseCallback(uv_handle_t *);
static void onInterfaceCallback(uv_poll_t *, int, int);
static void setPortParameters(struct Serial *, const struct SerialConfig *);
static bool setPortRate(struct Serial *, uint32_t);
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *, const void *);
static void streamDeinit(void *);
static void streamSetCallback(void *, void (*)(void *), void *);
static enum Result streamGetParam(void *, int, void *);
static enum Result streamSetParam(void *, int, const void *);
static size_t streamRead(void *, void *, size_t);
static size_t streamWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const Serial = &(const struct InterfaceClass){
    .size = sizeof(struct Serial),
    .init = streamInit,
    .deinit = streamDeinit,

    .setCallback = streamSetCallback,
    .getParam = streamGetParam,
    .setParam = streamSetParam,
    .read = streamRead,
    .write = streamWrite
};
/*----------------------------------------------------------------------------*/
static bool changePortFlag(struct Serial *interface, int flag, uint8_t state)
{
  const int cmd = state != 0 ? TIOCMBIS : TIOCMBIC;
  return ioctl(interface->descriptor, cmd, &flag) != -1;
}
/*----------------------------------------------------------------------------*/
static bool getPortFlag(struct Serial *interface, int flag, uint8_t *state)
{
  int value;

  if (ioctl(interface->descriptor, TIOCMGET, &value) != -1)
  {
    *state = (value & flag) ? 1 : 0;
    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static bool getPortParity(struct Serial *interface, uint8_t *parity)
{
  struct termios settings;

  if (tcgetattr(interface->descriptor, &settings) != -1)
  {
    if (!(settings.c_cflag & PARENB))
      *parity = SERIAL_PARITY_NONE;
    else if (!(settings.c_cflag & PARODD))
      *parity = SERIAL_PARITY_EVEN;
    else
      *parity = SERIAL_PARITY_ODD;

    return true;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static bool getPortRate(struct Serial *interface, uint32_t *actualRate)
{
  struct termios settings;

  if (tcgetattr(interface->descriptor, &settings) != -1)
  {
    const speed_t encodedRate = cfgetispeed(&settings);

    for (size_t index = 0; index < ARRAY_SIZE(rateList); ++index)
    {
      if (rateList[index].value == encodedRate)
      {
        *actualRate = rateList[index].key;
        return true;
      }
    }
  }

  return false;
}
/*----------------------------------------------------------------------------*/
static void onCloseCallback(uv_handle_t *handle)
{
  free(handle);
}
/*----------------------------------------------------------------------------*/
static void onInterfaceCallback(uv_poll_t *handle, int, int)
{
  struct Serial * const interface = uv_handle_get_data((uv_handle_t *)handle);
  uint8_t buffer[BUFFER_SIZE];
  ssize_t length;

  /* TODO Disconnect handling */
  pthread_mutex_lock(&interface->rxQueueLock);
  while ((length = read(interface->descriptor, buffer, sizeof(buffer))) > 0)
    byteQueuePushArray(&interface->rxQueue, buffer, (size_t)length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  if (interface->callback != NULL)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setPortParameters(struct Serial *interface,
    const struct SerialConfig *config)
{
  struct termios settings;

  tcgetattr(interface->descriptor, &settings);
  interface->initialSettings = settings;

  /* Enable raw mode, 8N1 */
  settings.c_iflag &=
      ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  settings.c_iflag |= INPCK | IGNPAR | IGNBRK;
  settings.c_oflag &= ~OPOST;
  settings.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  settings.c_cflag |= CLOCAL | CREAD;
  settings.c_cflag = (settings.c_cflag & ~CSIZE) | CS8;

  settings.c_cc[VMIN] = 0;  /* Minimal data packet length */
  settings.c_cc[VTIME] = 0; /* Time to wait for data */

  switch (config->parity)
  {
    case SERIAL_PARITY_ODD:
      settings.c_cflag |= PARENB;  /* Use parity */
      settings.c_cflag |= PARODD;  /* Odd parity */
      break;

    case SERIAL_PARITY_EVEN:
      settings.c_cflag |= PARENB;  /* Use parity */
      settings.c_cflag &= ~PARODD; /* Even parity */
      break;

    default:
      settings.c_cflag &= ~PARENB; /* Disable parity */
      break;
  }

  tcsetattr(interface->descriptor, TCSANOW, &settings);

  /* Configure baud rate */
  setPortRate(interface, config->rate);
}
/*----------------------------------------------------------------------------*/
static bool setPortRate(struct Serial *interface, uint32_t desiredRate)
{
  speed_t rate = 0;

  for (size_t index = 0; index < ARRAY_SIZE(rateList); ++index)
  {
    if (rateList[index].key == desiredRate)
    {
      rate = rateList[index].value;
      break;
    }
  }
  if (!rate)
    rate = desiredRate;

  struct termios settings;

  /* Update baud rate settings, initial configuration must be already saved */
  if (tcgetattr(interface->descriptor, &settings) != -1)
  {
    cfsetispeed(&settings, rate);
    cfsetospeed(&settings, rate);

    return tcsetattr(interface->descriptor, TCSANOW, &settings) != -1;
  }
  else
    return false;
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct SerialConfig * const config = configBase;
  struct Serial * const interface = object;
  enum Result res;

  interface->callback = NULL;

  if (pthread_mutex_init(&interface->rxQueueLock, 0))
    return E_ERROR;

  interface->listener = malloc(sizeof(uv_poll_t));
  if (interface->listener == NULL)
  {
    res = E_MEMORY;
    goto free_mutex;
  }

  if (!byteQueueInit(&interface->rxQueue, QUEUE_SIZE))
  {
    res = E_MEMORY;
    goto free_listener;
  }

  interface->descriptor = open(config->device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (interface->descriptor == -1)
  {
    res = E_INTERFACE;
    goto free_queue;
  }

  fcntl(interface->descriptor, F_SETFL, 0);
  setPortParameters(interface, config);

  uv_poll_init(uv_default_loop(), interface->listener, interface->descriptor);
  uv_handle_set_data((uv_handle_t *)interface->listener, interface);
  uv_poll_start(interface->listener, UV_READABLE, onInterfaceCallback);

  return E_OK;

free_queue:
  byteQueueDeinit(&interface->rxQueue);
free_listener:
  free(interface->listener);
free_mutex:
  pthread_mutex_destroy(&interface->rxQueueLock);
  return res;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object)
{
  struct Serial * const interface = object;

  uv_handle_set_data((uv_handle_t *)interface->listener, NULL);
  uv_close((uv_handle_t *)interface->listener, onCloseCallback);

  /* Restore terminal settings and close device */
  tcsetattr(interface->descriptor, TCSANOW, &interface->initialSettings);
  close(interface->descriptor);

  byteQueueDeinit(&interface->rxQueue);
  pthread_mutex_destroy(&interface->rxQueueLock);
}
/*----------------------------------------------------------------------------*/
static void streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Serial * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result streamGetParam(void *object, int parameter, void *data)
{
  struct Serial * const interface = object;

  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_PARITY:
      return getPortParity(interface, data) ? E_OK : E_INTERFACE;

    case IF_SERIAL_CTS:
      return getPortFlag(interface, TIOCM_CTS, data) ? E_OK : E_INTERFACE;

    case IF_SERIAL_RTS:
      return getPortFlag(interface, TIOCM_RTS, data) ? E_OK : E_INTERFACE;

    case IF_SERIAL_DSR:
      return getPortFlag(interface, TIOCM_DSR, data) ? E_OK : E_INTERFACE;

    case IF_SERIAL_DTR:
      return getPortFlag(interface, TIOCM_DTR, data) ? E_OK : E_INTERFACE;

    default:
      break;
  }

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

    case IF_RATE:
      return getPortRate(interface, data) ? E_OK : E_INTERFACE;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamSetParam(void *object, int parameter, const void *data)
{
  struct Serial * const interface = object;

  switch ((enum SerialParameter)parameter)
  {
    case IF_SERIAL_RTS:
      return changePortFlag(interface, TIOCM_RTS, *(const uint8_t *)data) ?
          E_OK : E_INTERFACE;

    case IF_SERIAL_DTR:
      return changePortFlag(interface, TIOCM_DTR, *(const uint8_t *)data) ?
          E_OK : E_INTERFACE;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_RATE:
      return setPortRate(interface, *(const uint32_t *)data) ?
          E_OK : E_INTERFACE;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t streamRead(void *object, void *buffer, size_t length)
{
  struct Serial * const interface = object;

  pthread_mutex_lock(&interface->rxQueueLock);
  const size_t read = byteQueuePopArray(&interface->rxQueue, buffer, length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  return read;
}
/*----------------------------------------------------------------------------*/
static size_t streamWrite(void *object, const void *buffer, size_t length)
{
  struct Serial * const interface = object;
  const ssize_t result = write(interface->descriptor, buffer, length);

  if (result != -1)
    return result;
  else
    return 0;
}
