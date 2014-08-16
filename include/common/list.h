/*
 * list.h
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#ifndef LIST_H_
#define LIST_H_
/*----------------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include <error.h>
/*----------------------------------------------------------------------------*/
struct ListNode
{
  /** Pointer to the next element or to an end of the list. */
  struct ListNode *next;
  /** Beginning of the user data. */
  unsigned char data[];
};
/*----------------------------------------------------------------------------*/
struct List
{
  /** First element of the list containing data nodes. */
  struct ListNode *first;
  /** First element of the free nodes list. */
  struct ListNode *pool;
  /** Size of each element in bytes. */
  unsigned int width;
};
/*----------------------------------------------------------------------------*/
enum result listInit(struct List *, unsigned int);
void listDeinit(struct List *);
void listClear(struct List *);
struct ListNode *listErase(struct List *, struct ListNode *);
enum result listPush(struct List *, const void *);

unsigned int listCapacity(const struct List *);
unsigned int listSize(const struct List *);
/*----------------------------------------------------------------------------*/
static inline void listData(const struct List *list,
    const struct ListNode *node, void *element)
{
  memcpy(element, node->data, list->width);
}
/*----------------------------------------------------------------------------*/
static inline struct ListNode *listFirst(const struct List *list)
{
  return list->first;
}
/*----------------------------------------------------------------------------*/
static inline struct ListNode *listNext(const struct ListNode *node)
{
  return node->next;
}
/*----------------------------------------------------------------------------*/
static inline bool listEmpty(const struct List *list)
{
  return list->first == 0;
}
/*----------------------------------------------------------------------------*/
#endif /* LIST_H_ */
