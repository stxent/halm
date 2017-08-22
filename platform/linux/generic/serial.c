/*
 * serial.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <ev.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <xcore/bits.h>
#include <xcore/containers/byte_queue.h>
#include <halm/platform/linux/serial.h>
/*----------------------------------------------------------------------------*/
#define BUFFER_SIZE 64
#define QUEUE_SIZE  2048
/*----------------------------------------------------------------------------*/
struct StreamRateEntry
{
  uint32_t key;
  speed_t value;
};

struct InterfaceWatcher
{
  ev_io io;
  void *instance;
};

struct Serial
{
  struct Interface parent;

  void (*callback)(void *);
  void *callbackArgument;

  struct ByteQueue rxQueue;
  pthread_mutex_t rxQueueLock;

  struct InterfaceWatcher watcher;
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
static void interfaceCallback(EV_P_ ev_io *, int);
static void setPortParameters(int, const struct SerialConfig *);
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
const struct InterfaceClass * const Serial = &streamTable;
/*----------------------------------------------------------------------------*/
static void interfaceCallback(EV_P_ ev_io *w,
    int revents __attribute__((unused)))
{
  struct InterfaceWatcher * const watcher = (struct InterfaceWatcher *)w;
  struct Serial * const interface = watcher->instance;
  uint8_t buffer[BUFFER_SIZE];
  ssize_t length;

  pthread_mutex_lock(&interface->rxQueueLock);
  while ((length = read(interface->descriptor, buffer, sizeof(buffer))) > 0)
    byteQueuePushArray(&interface->rxQueue, buffer, (size_t)length);
  pthread_mutex_unlock(&interface->rxQueueLock);

  if (interface->callback)
    interface->callback(interface->callbackArgument);
}
/*----------------------------------------------------------------------------*/
static void setPortParameters(int descriptor, const struct SerialConfig *config)
{
  speed_t rate = 0;

  for (size_t index = 0; index < ARRAY_SIZE(rateList); ++index)
  {
    if (rateList[index].value == config->rate)
    {
      rate = rateList[index].value;
      break;
    }
  }
  if (!rate)
    rate = config->rate;

  struct termios options;
  tcgetattr(descriptor, &options);

  /* Enable raw mode */
  options.c_iflag &= ~(BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  options.c_iflag |= INPCK | IGNPAR | IGNBRK;
  options.c_oflag &= ~OPOST;
  options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  options.c_cflag |= CLOCAL | CREAD;
  options.c_cflag = (options.c_cflag & ~CSIZE) | CS8;

  options.c_cc[VMIN] = 0; /* Minimal data packet length */
  options.c_cc[VTIME] = 1; /* Time to wait for data */

  switch (config->parity)
  {
    case SERIAL_PARITY_ODD:
      options.c_cflag |= PARENB; /* Use parity */
      options.c_cflag |= PARODD; /* Odd parity */
      break;

    case SERIAL_PARITY_EVEN:
      options.c_cflag |= PARENB; /* Use parity */
      options.c_cflag &= ~PARODD; /* Even parity */
      break;

    default:
      options.c_cflag &= ~PARENB; /* Disable parity */
      break;
  }

  cfsetispeed(&options, rate);
  cfsetospeed(&options, rate);

  tcsetattr(descriptor, TCSANOW, &options);
}
/*----------------------------------------------------------------------------*/
static enum Result streamInit(void *object, const void *configBase)
{
  const struct SerialConfig * const config = configBase;
  struct Serial * const interface = object;
  enum Result res;

  interface->callback = 0;

  if (pthread_mutex_init(&interface->rxQueueLock, 0))
    return E_ERROR;

  if ((res = byteQueueInit(&interface->rxQueue, QUEUE_SIZE)) != E_OK)
    goto free_mutex;

  interface->descriptor = open(config->device, O_RDWR | O_NOCTTY | O_NDELAY);
  if (interface->descriptor == -1)
  {
    res = E_INTERFACE;
    goto free_queue;
  }

  fcntl(interface->descriptor, F_SETFL, 0);
  setPortParameters(interface->descriptor, config);

  interface->watcher.instance = interface;
  ev_init(&interface->watcher.io, interfaceCallback);
  ev_io_set(&interface->watcher.io, interface->descriptor, EV_READ);
  ev_io_start(ev_default_loop(0), &interface->watcher.io);

  return E_OK;

free_queue:
  byteQueueDeinit(&interface->rxQueue);
free_mutex:
  pthread_mutex_destroy(&interface->rxQueueLock);
  return res;
}
/*----------------------------------------------------------------------------*/
static void streamDeinit(void *object)
{
  struct Serial * const interface = object;

  ev_io_stop(ev_default_loop(0), &interface->watcher.io);
  close(interface->descriptor);

  byteQueueDeinit(&interface->rxQueue);
  pthread_mutex_destroy(&interface->rxQueueLock);
}
/*----------------------------------------------------------------------------*/
static enum Result streamSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct Serial * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
  return E_OK;
}
/*----------------------------------------------------------------------------*/
static enum Result streamGetParam(void *object, enum IfParameter option,
    void *data)
{
  struct Serial * const interface = object;

  switch (option)
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
      break;
  }

  switch ((enum SerialOption)option)
  {
    case IF_SERIAL_CTS:
    {
      int value;

      if (ioctl(interface->descriptor, TIOCMGET, &value) != -1)
      {
        *(unsigned int *)data = (value & TIOCM_CTS) ? 1 : 0;
        return E_OK;
      }
      else
      {
        return E_INTERFACE;
      }
    }

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result streamSetParam(void *object, enum IfParameter option,
    const void *data)
{
  struct Serial * const interface = object;

  switch ((enum SerialOption)option)
  {
    case IF_SERIAL_RTS:
    {
      int value;

      if (ioctl(interface->descriptor, TIOCMGET, &value) == -1)
        return E_INTERFACE;

      if (*(const unsigned int *)data)
        value |= TIOCM_RTS;
      else
        value &= ~TIOCM_RTS;

      if (ioctl(interface->descriptor, TIOCMSET, &value) == -1)
        return E_INTERFACE;
      else
        return E_OK;
    }

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
