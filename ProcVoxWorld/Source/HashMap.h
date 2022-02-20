#pragma once

/* Uncomment if this is not already included in the file where this is included.
 * #include "Log.h"
 *
 * #include <stdint.h> */


#define HASH_MAP_DECLARATION(TYPE, TYPENAME)                             \
                                                                         \
typedef struct LinkedListNodeMap##TYPENAME                               \
{                                                                        \
  TYPE data;                                                             \
  struct LinkedListNodeMap##TYPENAME* ptrNext;                           \
} LinkedListNodeMap##TYPENAME;                                           \
                                                                         \
typedef struct                                                           \
{                                                                        \
  LinkedListNodeMap##TYPENAME *head, *tail;                              \
  size_t size;                                                           \
} LinkedListMap##TYPENAME;                                               \
                                                                         \
typedef struct                                                           \
{                                                                        \
  LinkedListMap##TYPENAME** array;                                       \
                                                                         \
  size_t arraySize;                                                      \
  size_t size;                                                           \
} HashMap##TYPENAME;                                                     \
                                                                         \
void HashMap##TYPENAME##Insert(HashMap##TYPENAME* map, TYPE elem);       \
int32_t HashMap##TYPENAME##Remove(HashMap##TYPENAME* map, TYPE elem);    \
int32_t HashMap##TYPENAME##Contains(HashMap##TYPENAME* map, TYPE elem);  \
void HashMap##TYPENAME##Delete(HashMap##TYPENAME* map);


#define HASH_MAP_IMPLEMENTATION(TYPE, TYPENAME, HASH_FUNC)                                                        \
                                                                                                                  \
HashMap##TYPENAME* HashMap##TYPENAME##Create(size_t arraySize)                                                    \
{                                                                                                                 \
  HashMap##TYPENAME* map = (HashMap##TYPENAME*)OwnMalloc(arraySize * sizeof(HashMap##TYPENAME), false);           \
                                                                                                                  \
  if(map == NULL)                                                                                                 \
  {                                                                                                               \
    LogError("Variable \"map\" in function \"HashMap##TYPENAME##Create\" "                                        \
             "from file \"HashMap.h\" must not be \"NULL\".", true);                                              \
                                                                                                                  \
    return NULL;                                                                                                  \
  }                                                                                                               \
                                                                                                                  \
  map->array = (LinkedListMap##TYPENAME**)OwnMalloc(arraySize * sizeof(LinkedListMap##TYPENAME*), false);         \
                                                                                                                  \
  if(map->array == NULL)                                                                                          \
  {                                                                                                               \
    LogError("Variable \"map->array\" in function \"HashMap##TYPENAME##Create\" "                                 \
             "from file \"HashMap.h\" must not be \"NULL\".", true);                                              \
                                                                                                                  \
    return NULL;                                                                                                  \
  }                                                                                                               \
                                                                                                                  \
  for(uint32_t i = 0; i < arraySize; ++i)                                                                         \
  {                                                                                                               \
    LinkedListMap##TYPENAME* list = (LinkedListMap##TYPENAME*)OwnMalloc(sizeof(LinkedListMap##TYPENAME), false);  \
                                                                                                                  \
    if(list == NULL)                                                                                              \
    {                                                                                                             \
      LogError("Variable \"list\" in function \"HashMap##TYPENAME##Create\" "                                     \
               "from file \"HashMap.h\" must not be \"NULL\".", true);                                            \
                                                                                                                  \
      return NULL;                                                                                                \
    }                                                                                                             \
                                                                                                                  \
    list->head = NULL;                                                                                            \
    list->tail = NULL;                                                                                            \
    list->size = 0;                                                                                               \
    map->array[i] = list;                                                                                         \
  }                                                                                                               \
                                                                                                                  \
  map->arraySize = arraySize;                                                                                     \
  map->size = 0;                                                                                                  \
                                                                                                                  \
  return map;                                                                                                     \
}                                                                                                                 \
                                                                                                                  \
void HashMap##TYPENAME##Insert(HashMap##TYPENAME* map, TYPE elem)                                                 \
{                                                                                                                 \
  uint32_t index = HASH_FUNC(elem) % map->arraySize;                                                              \
  LinkedListMap##TYPENAME* list = map->array[index];                                                              \
                                                                                                                  \
  LinkedListNodeMap##TYPENAME* node = calloc(1, sizeof(LinkedListNodeMap##TYPENAME));                             \
                                                                                                                  \
  if(node == NULL)                                                                                                \
  {                                                                                                               \
    LogError("Variable \"node\" in function \"HashMap##TYPENAME##Insert\" "                                       \
             "from file \"HashMap.h\" must not be \"NULL\".", true);                                              \
                                                                                                                  \
    return;                                                                                                       \
  }                                                                                                               \
                                                                                                                  \
  node->data = elem;                                                                                              \
  node->ptrNext = NULL;                                                                                           \
                                                                                                                  \
  if(!list->size)                                                                                                 \
  {                                                                                                               \
    list->head = node;                                                                                            \
    list->tail = node;                                                                                            \
  }                                                                                                               \
  else                                                                                                            \
  {                                                                                                               \
    LinkedListNodeMap##TYPENAME* prevHead = list->head;                                                           \
    list->head = node;                                                                                            \
    list->head->ptrNext = prevHead;                                                                               \
  }                                                                                                               \
                                                                                                                  \
  ++list->size;                                                                                                   \
  ++map->size;                                                                                                    \
}                                                                                                                 \
                                                                                                                  \
int32_t HashMap##TYPENAME##Remove(HashMap##TYPENAME* map, TYPE elem)                                              \
{                                                                                                                 \
  uint32_t index = HASH_FUNC(elem) % map->arraySize;                                                              \
  LinkedListMap##TYPENAME* list = map->array[index];                                                              \
                                                                                                                  \
  if(!list->size)                                                                                                 \
    return 0;                                                                                                     \
                                                                                                                  \
  if(list->size == 1)                                                                                             \
  {                                                                                                               \
    if(list->head->data != elem)                                                                                  \
      return 0;                                                                                                   \
                                                                                                                  \
    free(list->head);                                                                                             \
    list->head = NULL;                                                                                            \
    list->tail = NULL;                                                                                            \
    list->size = 0;                                                                                               \
    --map->size;                                                                                                  \
                                                                                                                  \
    return 1;                                                                                                     \
  }                                                                                                               \
  else                                                                                                            \
  {                                                                                                               \
    if(list->head->data == elem)                                                                                  \
    {                                                                                                             \
      LinkedListNodeMap##TYPENAME* prevHead = list->head;                                                         \
      list->head = list->head->ptrNext;                                                                           \
      --list->size;                                                                                               \
      --map->size;                                                                                                \
      free(prevHead);                                                                                             \
                                                                                                                  \
      return 1;                                                                                                   \
    }                                                                                                             \
                                                                                                                  \
    LinkedListNodeMap##TYPENAME* currNode = list->head;                                                           \
    while(currNode->ptrNext && currNode->ptrNext->data != elem)                                                   \
      currNode = currNode->ptrNext;                                                                               \
                                                                                                                  \
    if(!currNode->ptrNext)                                                                                        \
      return 0;                                                                                                   \
                                                                                                                  \
    if(currNode->ptrNext == list->tail)                                                                           \
    {                                                                                                             \
      LinkedListNodeMap##TYPENAME* prevTail = list->tail;                                                         \
      list->tail = currNode;                                                                                      \
      list->tail->ptrNext = NULL;                                                                                 \
      free(prevTail);                                                                                             \
    }                                                                                                             \
    else                                                                                                          \
    {                                                                                                             \
      LinkedListNodeMap##TYPENAME* nodeToDel = currNode->ptrNext;                                                 \
      currNode->ptrNext = currNode->ptrNext->ptrNext;                                                             \
      free(nodeToDel);                                                                                            \
    }                                                                                                             \
                                                                                                                  \
    --list->size;                                                                                                 \
    --map->size;                                                                                                  \
                                                                                                                  \
    return 1;                                                                                                     \
  }                                                                                                               \
}                                                                                                                 \
                                                                                                                  \
int32_t HashMap##TYPENAME##Contains(HashMap##TYPENAME* map, TYPE elem)                                            \
{                                                                                                                 \
  uint32_t index = HASH_FUNC(elem) % map->arraySize;                                                              \
  LinkedListMap##TYPENAME* list = map->array[index];                                                              \
                                                                                                                  \
  if(list->size == 0)                                                                                             \
    return 0;                                                                                                     \
                                                                                                                  \
  if(list->head->data == elem || list->tail->data == elem)                                                        \
    return 1;                                                                                                     \
                                                                                                                  \
  LinkedListNodeMap##TYPENAME* currNode = list->head;                                                             \
  while(currNode && currNode->data != elem)                                                                       \
    currNode = currNode->ptrNext;                                                                                 \
                                                                                                                  \
  return currNode ? 1 : 0;                                                                                        \
}                                                                                                                 \
                                                                                                                  \
void HashMap##TYPENAME##Delete(HashMap##TYPENAME* map)                                                            \
{                                                                                                                 \
  for(uint32_t i = 0; i < map->arraySize; ++i)                                                                    \
  {                                                                                                               \
    LinkedListMap##TYPENAME* list = map->array[i];                                                                \
    LinkedListNodeMap##TYPENAME* currNode = list->head;                                                           \
                                                                                                                  \
    while(currNode)                                                                                               \
    {                                                                                                             \
      LinkedListNodeMap##TYPENAME* nextNode = currNode->ptrNext;                                                  \
      free(currNode);                                                                                             \
      currNode = nextNode;                                                                                        \
    }                                                                                                             \
                                                                                                                  \
    free(list);                                                                                                   \
  }                                                                                                               \
                                                                                                                  \
  free(map->array);                                                                                               \
}