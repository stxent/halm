/*
 * ram_proxy.c
 * Copyright (C) 2023 xent
 * Project is distributed under the terms of the MIT License
 */

#include <halm/generic/flash.h>
#include <halm/generic/ram_proxy.h>
#include <assert.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define DEFAULT_GRANULE_SIZE 1024
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *, const void *);
static void interfaceSetCallback(void *, void (*)(void *), void *);
static enum Result interfaceGetParam(void *, int, void *);
static enum Result interfaceSetParam(void *, int, const void *);
static size_t interfaceRead(void *, void *, size_t);
static size_t interfaceWrite(void *, const void *, size_t);
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const RamProxy =
    &(const struct InterfaceClass){
    .size = sizeof(struct RamProxy),
    .init = interfaceInit,
    .deinit = NULL, /* Default destructor */

    .setCallback = interfaceSetCallback,
    .getParam = interfaceGetParam,
    .setParam = interfaceSetParam,
    .read = interfaceRead,
    .write = interfaceWrite
};
/*----------------------------------------------------------------------------*/
static enum Result interfaceInit(void *object, const void *configBase)
{
  const struct RamProxyConfig * const config = configBase;
  assert(config != NULL);
  assert(config->arena != NULL && config->capacity > 0);

  struct RamProxy * const interface = object;

  interface->callback = NULL;
  interface->arena = config->arena;
  interface->capacity = config->capacity;
  interface->granule = config->granule ? config->granule : DEFAULT_GRANULE_SIZE;
  interface->position = 0;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void interfaceSetCallback(void *object, void (*callback)(void *),
    void *argument)
{
  struct RamProxy * const interface = object;

  interface->callbackArgument = argument;
  interface->callback = callback;
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceGetParam(void *object, int parameter, void *data)
{
  struct RamProxy * const interface = object;

  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_SECTOR_SIZE:
      *(uint32_t *)data = (uint32_t)interface->granule;
      return E_OK;

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
      *(uint32_t *)data = (uint32_t)interface->position;
      return E_OK;

    case IF_POSITION_64:
      *(uint64_t *)data = interface->position;
      return E_OK;

    case IF_SIZE:
      *(uint32_t *)data = (uint32_t)interface->capacity;
      return E_OK;

    case IF_SIZE_64:
      *(uint64_t *)data = interface->capacity;
      return E_OK;

    case IF_STATUS:
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static enum Result interfaceSetParam(void *object, int parameter,
    const void *data)
{
  struct RamProxy * const interface = object;

  switch ((enum FlashParameter)parameter)
  {
    case IF_FLASH_ERASE_SECTOR:
    {
      const uint32_t address = *(const uint32_t *)data;

      if (address < interface->capacity && (address % interface->granule) == 0)
      {
        memset(interface->arena + address, 0, interface->granule);
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    default:
      break;
  }

  switch ((enum IfParameter)parameter)
  {
    case IF_POSITION:
    {
      const uint32_t address = *(const uint32_t *)data;

      if (address < interface->capacity)
      {
        interface->position = address;
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    case IF_POSITION_64:
    {
      const uint64_t address = *(const uint64_t *)data;

      if (address < interface->capacity)
      {
        interface->position = (uintptr_t)address;
        return E_OK;
      }
      else
        return E_ADDRESS;
    }

    case IF_BLOCKING:
      return E_OK;

    case IF_ZEROCOPY:
      return E_OK;

    default:
      return E_INVALID;
  }
}
/*----------------------------------------------------------------------------*/
static size_t interfaceRead(void *object, void *buffer, size_t length)
{
  struct RamProxy * const interface = object;

  if (interface->position + length > interface->capacity)
    length = interface->capacity - interface->position;
  memcpy(buffer, interface->arena + interface->position, length);

  return length;
}
/*----------------------------------------------------------------------------*/
static size_t interfaceWrite(void *object, const void *buffer, size_t length)
{
  struct RamProxy * const interface = object;

  if (interface->position + length > interface->capacity)
    length = interface->capacity - interface->position;
  memcpy(interface->arena + interface->position, buffer, length);

  return length;
}
