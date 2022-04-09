//
// Created by yao on 2022/2/18.
//

#include "Stack.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"


void *Stack_pop(Stack *self) {
    if (self->len == 0) return NULL;
    return self->p[--self->len];
}

void Stack_releaseNode(void *node) {
    if (node) {
        NodeBase *p = node;
        if (p->destructor)
            p->destructor(p);
        os_free(node);
    }
}

int Stack_init(Stack *self) {
    self->len = 0;
    self->p = NULL;
    return 1;
}

void *Stack_top(Stack *self) {
    return Array_get(self, self->len - 1);
}


int Stack_push(Stack *self, void *p, size_t element_size) {
    return Array_push(self, p, element_size);
}

void Stack_delete(Stack *self) {
    Array_delete(self);
}

