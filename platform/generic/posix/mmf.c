/*
 * mmf.c
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <xcore/memory.h>
#include <halm/platform/generic/mmf.h>
/*----------------------------------------------------------------------------*/
static enum Result mmfInit(void *, const void *);
static void mmfDeinit(void *);
static enum Result mmfSetCallback(void *, void (*)(void *), void *);
static enum Result mmfGetParam(void *, enum IfParameter, void *);
static enum Result mmfSetParam(void *, enum IfParameter, const void *);
static size_t mmfRead(void *, void *, size_t);
static size_t mmfWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
struct MemoryMappedFile
{
  struct Interface base;

  off_t position;
  off_t offset;
  off_t size;

  sem_t semaphore;

  struct stat info;
  uint8_t *data;
  int file;
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const MemoryMappedFile =
    &(const struct InterfaceClass){
    .size = sizeof(struct MemoryMappedFile),
    .init = mmfInit,
    .deinit = mmfDeinit,

    .setCallback = mmfSetCallback,
    .getParam = mmfGetParam,
    .setParam = mmfSetParam,
    .read = mmfRead,
    .write = mmfWrite
};
/*----------------------------------------------------------------------------*/
static enum Result mmfInit(void *object, const void *configBase)
{
  const char * const path = configBase;
  struct MemoryMappedFile * const dev = object;
  enum Result res;

  if (!path)
    return E_ERROR;

  dev->position = 0;
  dev->offset = 0;
  dev->size = 0;

  if (sem_init(&dev->semaphore, 0, 1))
    return E_ERROR;

  dev->file = open(path, O_RDWR);
  if (!dev->file)
  {
    res = E_ENTRY;
    goto free_semaphore;
  }
  if (fstat(dev->file, &(dev->info)) == -1 || dev->info.st_size == 0)
  {
    res = E_INTERFACE;
    goto free_file;
  }

  dev->data = mmap(0, dev->info.st_size, PROT_WRITE, MAP_SHARED, dev->file, 0);
  if (dev->data == MAP_FAILED)
  {
    res = E_INTERFACE;
    goto free_file;
  }

  dev->size = dev->info.st_size;
  return E_OK;

free_file:
  close(dev->file);
free_semaphore:
  sem_destroy(&dev->semaphore);
  return res;
}
/*----------------------------------------------------------------------------*/
static void mmfDeinit(void *object)
{
  struct MemoryMappedFile * const dev = object;

  munmap(dev->data, dev->info.st_size);
  close(dev->file);
  sem_destroy(&dev->semaphore);
}
/*----------------------------------------------------------------------------*/
static enum Result mmfSetCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  /* Not implemented */
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum Result mmfGetParam(void *object, enum IfParameter parameter,
    void *data)
{
  struct MemoryMappedFile * const dev = object;

  switch (parameter)
  {
    case IF_POSITION_64:
      *(uint64_t *)data = (uint64_t)dev->position;
      return E_OK;

    case IF_SIZE_64:
      *(uint64_t *)data = (uint64_t)dev->size;
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result mmfSetParam(void *object, enum IfParameter parameter,
    const void *data)
{
  struct MemoryMappedFile * const dev = object;

  switch (parameter)
  {
    case IF_POSITION_64:
    {
      const off_t position = (off_t)(*(const uint64_t *)data);

      if (position + dev->offset < dev->info.st_size)
      {
        dev->position = position;
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    case IF_ACQUIRE:
      sem_wait(&dev->semaphore);
      return E_OK;

    case IF_RELEASE:
      sem_post(&dev->semaphore);
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t mmfRead(void *object, void *buffer, size_t length)
{
  struct MemoryMappedFile * const dev = object;

  memcpy(buffer, dev->data + dev->offset + dev->position, length);
  dev->position += length;

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t mmfWrite(void *object, const void *buffer, size_t length)
{
  struct MemoryMappedFile * const dev = object;

  memcpy(dev->data + dev->offset + dev->position, buffer, length);
  dev->position += length;

  return length;
}
