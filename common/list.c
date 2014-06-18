/*
 * list.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <stdlib.h>
#include <list.h>
/*----------------------------------------------------------------------------*/
static unsigned int countListNodes(const struct ListNode *current)
{
  unsigned int result = 0;

  while (current)
  {
    ++result;
    current = current->next;
  }

  return result;
}
/*----------------------------------------------------------------------------*/
static void freeListChain(struct ListNode *current)
{
  struct ListNode *buffer;

  while (current)
  {
    buffer = current;
    current = current->next;
    free(buffer);
  }
}
/*----------------------------------------------------------------------------*/
enum result listInit(struct List *list, unsigned int width)
{
  list->first = list->pool = 0;
  list->width = width;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
void listDeinit(struct List *list)
{
  freeListChain(list->first);
  freeListChain(list->pool);
}
/*----------------------------------------------------------------------------*/
void listClear(struct List *list)
{
  if (list->first)
  {
    struct ListNode *current = list->first;

    while (current->next)
      current = current->next;

    current->next = list->pool;
    list->pool = list->first;
    list->first = 0;
  }
}
/*----------------------------------------------------------------------------*/
void *listErase(struct List *list, void *node)
{
  struct ListNode *next, *target = node;

  if (list->first != node)
  {
    struct ListNode *current = list->first;

    while (current->next != node)
      current = current->next;

    current->next = target->next;
  }
  else
    list->first = list->first->next;

  next = target->next;
  target->next = list->pool;
  list->pool = target;

  return next;
}
/*----------------------------------------------------------------------------*/
enum result listPush(struct List *list, const void *element)
{
  struct ListNode *node;

  if (list->pool)
  {
    node = list->pool;
    list->pool = list->pool->next;
  }
  else
  {
    node = malloc(sizeof(struct ListNode *) + list->width);

    if (!node)
      return E_MEMORY;
  }

  memcpy(node->data, element, list->width);
  node->next = 0;

  if (list->first)
  {
    struct ListNode *current = list->first;

    while (current->next)
      current = current->next;

    current->next = node;
  }
  else
    list->first = node;

  return E_OK;
}
/*----------------------------------------------------------------------------*/
unsigned int listCapacity(const struct List *list)
{
  return countListNodes(list->first) + countListNodes(list->pool);
}
/*----------------------------------------------------------------------------*/
unsigned int listSize(const struct List *list)
{
  return countListNodes(list->first);
}
