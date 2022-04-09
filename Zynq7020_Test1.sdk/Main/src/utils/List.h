//
// Created by yao on 2022/2/26.
//

#ifndef TELEPHONEDIRECTORY_LIST_H
#define TELEPHONEDIRECTORY_LIST_H

#include <stddef.h>
#include "NodeBase.h"

typedef struct ListNode {
    NodeBase *data;
    struct ListNode* last;
    struct ListNode* next;
} ListNode;

typedef struct {
    ListNode* head;
    ListNode* tail;
    size_t len;
} List;

int List_init(List* self);

int List_delete(List *self);

int List_insert(List *self, ListNode *before, void*data, size_t size);

int List_remove(List *self, ListNode *node, ListNode **next_ptr);

int List_pushHead(List *self, void *data, size_t size);

int List_pushBack(List *self, void *data, size_t size);

int List_switchNode(ListNode **node1, ListNode **node2);

int Link_sort(List *self, CompareFunction_t comp);

#endif //TELEPHONEDIRECTORY_LIST_H
