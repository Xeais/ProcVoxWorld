#pragma once

/* Uncomment if this is not already included in the file where this is included.
 * #include "Log.h"
 *
 * #include <stdint.h> */

#define LINKED_LIST_DECLARATION(TYPE, TYPENAME)                                 \
                                                                                \
typedef struct LinkedListNode##TYPENAME                                         \
{                                                                               \
  TYPE data;                                                                    \
  struct LinkedListNode##TYPENAME* ptrNext;                                     \
} LinkedListNode##TYPENAME;                                                     \
                                                                                \
typedef struct                                                                  \
{                                                                               \
  LinkedListNode##TYPENAME *head, *tail;                                        \
  size_t size;                                                                  \
} LinkedList##TYPENAME;                                                         \
                                                                                \
LinkedList##TYPENAME* LinkedList##TYPENAME##Create();                           \
void LinkedList##TYPENAME##PushBack(LinkedList##TYPENAME* list, TYPE elem);     \
void LinkedList##TYPENAME##PushFront(LinkedList##TYPENAME* list, TYPE elem);    \
TYPE LinkedList##TYPENAME##PopFront(LinkedList##TYPENAME* list);                \
int32_t LinkedList##TYPENAME##Remove(LinkedList##TYPENAME* list, TYPE elem);    \
int32_t LinkedList##TYPENAME##Contains(LinkedList##TYPENAME* list, TYPE elem);  \
TYPE LinkedList##TYPENAME##Get(LinkedList##TYPENAME* list, size_t index);       \
TYPE LinkedList##TYPENAME##Front(LinkedList##TYPENAME* list);                   \
TYPE LinkedList##TYPENAME##Back(LinkedList##TYPENAME* list);                    \
void LinkedList##TYPENAME##Clear(LinkedList##TYPENAME* list);                   \
void LinkedList##TYPENAME##Delete(LinkedList##TYPENAME* list);                


#define LINKED_LIST_IMPLEMENTATION(TYPE, TYPENAME)                                                                 \
                                                                                                                   \
LinkedList##TYPENAME* LinkedList##TYPENAME##Create()                                                               \
{                                                                                                                  \
  LinkedList##TYPENAME* list = (LinkedList##TYPENAME*)OwnMalloc(sizeof(LinkedList##TYPENAME), false);              \
                                                                                                                   \
  if(list == NULL)                                                                                                 \
  {                                                                                                                \
    LogError("Variable \"list\" in function \"LinkedList##TYPENAME##Create\" "                                     \
             "from file \"LinkedList.h\" must not be \"NULL\".", true);                                            \
                                                                                                                   \
    return NULL;                                                                                                   \
  }                                                                                                                \
                                                                                                                   \
  list->head = NULL;                                                                                               \
  list->tail = NULL;                                                                                               \
  list->size = 0;                                                                                                  \
                                                                                                                   \
  return list;                                                                                                     \
}                                                                                                                  \
                                                                                                                   \
void LinkedList##TYPENAME##PushBack(LinkedList##TYPENAME* list, TYPE elem)                                         \
{                                                                                                                  \
  LinkedListNode##TYPENAME* node = (LinkedListNode##TYPENAME*)OwnMalloc(sizeof(LinkedListNode##TYPENAME), false);  \
                                                                                                                   \
  if(node == NULL)                                                                                                 \
  {                                                                                                                \
    LogError("Variable \"node\" in function \"LinkedList##TYPENAME##PushBack\" "                                   \
             "from file \"LinkedList.h\" must not be \"NULL\".", true);                                            \
                                                                                                                   \
    return;                                                                                                        \
  }                                                                                                                \
                                                                                                                   \
  node->data = elem;                                                                                               \
  node->ptrNext = NULL;                                                                                            \
                                                                                                                   \
  if(!list->head)                                                                                                  \
  {                                                                                                                \
    list->head = node;                                                                                             \
    list->tail = node;                                                                                             \
  }                                                                                                                \
  else                                                                                                             \
  {                                                                                                                \
    LinkedListNode##TYPENAME* prevTail = list->tail;                                                               \
    list->tail = node;                                                                                             \
    prevTail->ptrNext = node;                                                                                      \
  }                                                                                                                \
                                                                                                                   \
  ++list->size;                                                                                                    \
}                                                                                                                  \
                                                                                                                   \
void LinkedList##TYPENAME##PushFront(LinkedList##TYPENAME* list, TYPE elem)                                        \
{                                                                                                                  \
  LinkedListNode##TYPENAME* node = (LinkedListNode##TYPENAME*)OwnMalloc(sizeof(LinkedListNode##TYPENAME), false);  \
                                                                                                                   \
  if(node == NULL)                                                                                                 \
  {                                                                                                                \
    LogError("Variable \"node\" in function \"LinkedList##TYPENAME##PushFront\" "                                  \
             "from file \"LinkedList.h\" must not be \"NULL\".", true);                                            \
                                                                                                                   \
    return;                                                                                                        \
  }                                                                                                                \
                                                                                                                   \
  node->data = elem;                                                                                               \
  node->ptrNext = NULL;                                                                                            \
                                                                                                                   \
  if(!list->size)                                                                                                  \
  {                                                                                                                \
    list->head = node;                                                                                             \
    list->tail = node;                                                                                             \
  }                                                                                                                \
  else                                                                                                             \
  {                                                                                                                \
    LinkedListNode##TYPENAME* prevHead = list->head;                                                               \
    list->head = node;                                                                                             \
    list->head->ptrNext = prevHead;                                                                                \
  }                                                                                                                \
                                                                                                                   \
  ++list->size;                                                                                                    \
}                                                                                                                  \
                                                                                                                   \
TYPE LinkedList##TYPENAME##PopFront(LinkedList##TYPENAME* list)                                                    \
{                                                                                                                  \
  if(list->size == 0)                                                                                              \
    return (TYPE)0;                                                                                                \
                                                                                                                   \
  if(list->size == 1)                                                                                              \
  {                                                                                                                \
    TYPE elem = list->head->data;                                                                                  \
    free(list->head);                                                                                              \
    list->head = NULL;                                                                                             \
    list->tail = NULL;                                                                                             \
    list->size = 0;                                                                                                \
                                                                                                                   \
    return elem;                                                                                                   \
  }                                                                                                                \
                                                                                                                   \
  TYPE elem = list->head->data;                                                                                    \
  LinkedListNode##TYPENAME* prevHead = list->head;                                                                 \
  list->head = list->head->ptrNext;                                                                                \
  --list->size;                                                                                                    \
  free(prevHead);                                                                                                  \
                                                                                                                   \
  return elem;                                                                                                     \
}                                                                                                                  \
                                                                                                                   \
int32_t LinkedList##TYPENAME##Remove(LinkedList##TYPENAME* list, TYPE elem)                                        \
{                                                                                                                  \
  if(!list->size)                                                                                                  \
    return 0;                                                                                                      \
                                                                                                                   \
  if(list->size == 1)                                                                                              \
  {                                                                                                                \
    if(list->head->data != elem)                                                                                   \
      return 0;                                                                                                    \
                                                                                                                   \
    free(list->head);                                                                                              \
    list->head = NULL;                                                                                             \
    list->tail = NULL;                                                                                             \
    list->size = 0;                                                                                                \
                                                                                                                   \
    return 1;                                                                                                      \
  }                                                                                                                \
  else                                                                                                             \
  {                                                                                                                \
    if(list->head->data == elem)                                                                                   \
    {                                                                                                              \
      LinkedListNode##TYPENAME* prevHead = list->head;                                                             \
      list->head = list->head->ptrNext;                                                                            \
      --list->size;                                                                                                \
      free(prevHead);                                                                                              \
                                                                                                                   \
      return 1;                                                                                                    \
    }                                                                                                              \
                                                                                                                   \
    LinkedListNode##TYPENAME* currNode = list->head;                                                               \
    while(currNode->ptrNext && currNode->ptrNext->data != elem)                                                    \
      currNode = currNode->ptrNext;                                                                                \
                                                                                                                   \
    if(!currNode->ptrNext)                                                                                         \
      return 0;                                                                                                    \
                                                                                                                   \
    if(currNode->ptrNext == list->tail)                                                                            \
    {                                                                                                              \
      LinkedListNode##TYPENAME* prevTail = list->tail;                                                             \
      list->tail = currNode;                                                                                       \
      list->tail->ptrNext = NULL;                                                                                  \
      free(prevTail);                                                                                              \
    }                                                                                                              \
    else                                                                                                           \
    {                                                                                                              \
      LinkedListNode##TYPENAME* nodeToDel = currNode->ptrNext;                                                     \
      currNode->ptrNext = currNode->ptrNext->ptrNext;                                                              \
      free(nodeToDel);                                                                                             \
    }                                                                                                              \
                                                                                                                   \
    --list->size;                                                                                                  \
                                                                                                                   \
    return 1;                                                                                                      \
  }                                                                                                                \
}                                                                                                                  \
                                                                                                                   \
int32_t LinkedList##TYPENAME##Contains(LinkedList##TYPENAME* list, TYPE elem)                                      \
{                                                                                                                  \
  if(list->size == 0)                                                                                              \
    return 0;                                                                                                      \
                                                                                                                   \
  if(list->head->data == elem || list->tail->data == elem)                                                         \
    return 1;                                                                                                      \
                                                                                                                   \
  LinkedListNode##TYPENAME* currNode = list->head;                                                                 \
  while(currNode && currNode->data != elem)                                                                        \
    currNode = currNode->ptrNext;                                                                                  \
                                                                                                                   \
  return currNode ? 1 : 0;                                                                                         \
}                                                                                                                  \
                                                                                                                   \
TYPE LinkedList##TYPENAME##Get(LinkedList##TYPENAME* list, size_t index)                                           \
{                                                                                                                  \
  if(index + 1 >= list->size)                                                                                      \
    return (TYPE)0;                                                                                                \
                                                                                                                   \
  if(index == 0)                                                                                                   \
    return list->head->data;                                                                                       \
  else if(index == list->size - 1)                                                                                 \
    return list->tail->data;                                                                                       \
  else                                                                                                             \
  {                                                                                                                \
    LinkedListNode##TYPENAME* currNode = list->head;                                                               \
    while(index--)                                                                                                 \
      currNode = currNode->ptrNext;                                                                                \
                                                                                                                   \
    return currNode->data;                                                                                         \
  }                                                                                                                \
}                                                                                                                  \
                                                                                                                   \
TYPE LinkedList##TYPENAME##Front(LinkedList##TYPENAME* list)                                                       \
{                                                                                                                  \
  if(!list->size)                                                                                                  \
    return (TYPE)0;                                                                                                \
                                                                                                                   \
  return list->head->data;                                                                                         \
}                                                                                                                  \
                                                                                                                   \
TYPE LinkedList##TYPENAME##Back(LinkedList##TYPENAME* list)                                                        \
{                                                                                                                  \
  if(!list->size)                                                                                                  \
    return (TYPE)0;                                                                                                \
                                                                                                                   \
  return list->tail->data;                                                                                         \
}                                                                                                                  \
                                                                                                                   \
void LinkedList##TYPENAME##Clear(LinkedList##TYPENAME* list)                                                       \
{                                                                                                                  \
  LinkedListNode##TYPENAME* currNode = list->head;                                                                 \
                                                                                                                   \
  while(currNode)                                                                                                  \
  {                                                                                                                \
    LinkedListNode##TYPENAME* nextNode = currNode->ptrNext;                                                        \
    free(currNode);                                                                                                \
    currNode = nextNode;                                                                                           \
  }                                                                                                                \
                                                                                                                   \
  list->size = 0;                                                                                                  \
  list->head = NULL;                                                                                               \
  list->tail = NULL;                                                                                               \
}                                                                                                                  \
                                                                                                                   \
void LinkedList##TYPENAME##Delete(LinkedList##TYPENAME* list)                                                      \
{                                                                                                                  \
  LinkedList##TYPENAME##Clear(list);                                                                               \
  free(list);                                                                                                      \
}                                                                                