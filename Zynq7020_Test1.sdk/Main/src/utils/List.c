//
// Created by yao on 2022/2/26.
//

#include <string.h>
#include <stdlib.h>
#include "List.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"

int List_init(List *self) {
    if (self == NULL) return 0;
    memset(self, 0, sizeof(List));
    return 1;
}

static ListNode *Link_createNode(void *data, size_t dataSize) {
    if (data == NULL || dataSize == 0) return NULL;
    ListNode *new_node = os_malloc(sizeof(ListNode));
    void *data_p = os_malloc(dataSize);
    if (new_node != NULL && data_p != NULL) {
        new_node->next = new_node->last = NULL;
        new_node->data = data_p;
        memcpy(new_node->data, data, dataSize);
        new_node->data->size = dataSize;
        return new_node;
    } else {
        if (new_node) os_free(new_node);
        if (data_p) os_free(data_p);
        return NULL;
    }
}

int List_pushBack(List *self, void *data, size_t size) {
    if (self == NULL) return 0;
    ListNode *new_node = Link_createNode(data, size);
    if (new_node == NULL) return 0;

    if (self->len == 0) {
        self->head = self->tail = new_node;
    } else {
        self->tail->next = new_node;
        new_node->last = self->tail;
        self->tail = new_node;
    }
    self->len++;
    return 1;
}

int List_pushHead(List *self, void *data, size_t size) {
    if (self == NULL) return 0;
    ListNode *new_node = Link_createNode(data, size);
    if (new_node == NULL) return 0;

    if (self->len == 0) {
        self->head = self->tail = new_node;
    } else {
        self->head->last = new_node;
        new_node->next = self->head;
        self->head = new_node;
    }
    self->len++;
    return 1;
}

int List_remove(List *self, ListNode *node, ListNode **next_ptr) {
    if (node == NULL || self == NULL) return 0;
    ListNode *last_node = node->last;
    ListNode *next_node = node->next;

    if (self->head == node)
        self->head = next_node;

    if (self->tail == node)
        self->tail = last_node;

    if (last_node)
        last_node->next = next_node;

    if (next_node)
        next_node->last = last_node;

    if (node->data->destructor)
        node->data->destructor(node->data);
    os_free(node->data);
    os_free(node);
    self->len--;
    if (next_ptr) *next_ptr = next_node;
    return 1;
}

int List_insert(List *self, ListNode *before, void *data, size_t size) {
    if (before == NULL || self == NULL || data == NULL) return 0;
    ListNode *new_node = Link_createNode(data, size);
    if (new_node == NULL) return 0;
    ListNode *next_node = before->next;
    before->next = new_node;
    new_node->last = before;
    new_node->next = next_node;
    next_node->last = new_node;
    self->len++;
    return 1;
}


int List_delete(List *self) {
    ListNode *node = self->head;
    for (size_t i = 0; i < self->len; i++) {
        if (node->data->destructor)
            node->data->destructor(node->data);
        os_free(node->data);
        ListNode *next_node = node->next;
        os_free(node);
        node = next_node;
    }
    List_init(self);
    return 0;
}

static void switch_data(void **a, void **b) {
    void *buf = *a;
    *a = *b;
    *b = buf;
}

int List_switchNode(ListNode **node1, ListNode **node2) {
    if (node1 == NULL || node2 == NULL || *node1 == NULL || *node2 == NULL) return 0;
    if (node2 == node1) return 1;
    ListNode *node1_last = (*node1)->last;
    ListNode *node1_next = (*node1)->next;
    ListNode *node2_last = (*node2)->last;
    ListNode *node2_next = (*node2)->next;

    switch_data((void **) &(*node2)->last, (void **) &(*node1)->last);
    switch_data((void **) &(*node2)->next, (void **) &(*node1)->next);

    if (node1_last) node1_last->next = *node2;
    if (node1_next) node1_next->last = *node2;
    if (node2_last) node2_last->next = *node1;
    if (node2_next) node2_next->last = *node1;

    switch_data((void **) node1, (void **) node2);
    return 1;
}

int Link_sort(List *self, CompareFunction_t comp) {
    if (comp == NULL || self == NULL) return 0;
    List new_list = { 0 };

    while (self->len) {
        ListNode *flag_node = self->head;
        ListNode *node = self->head;
        while (node) {
            if (comp(flag_node->data, node->data))
                flag_node = node;
            node = node->next;
        }
        if (self->head == flag_node) self->head = flag_node->next;
        if (flag_node->last) flag_node->last->next = flag_node->next;
        if (flag_node->next) flag_node->next->last = flag_node->last;
        if (new_list.len == 0) {
            flag_node->last = flag_node->next = NULL;
            new_list.head = new_list.tail = flag_node;
        } else {
            new_list.tail->next = flag_node;
            new_list.tail = flag_node;
        }
        new_list.len++;
        self->len--;
    }
    *self = new_list;
    return 1;
}
