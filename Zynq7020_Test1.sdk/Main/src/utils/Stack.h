//
// Created by yao on 2022/2/18.
//

#ifndef CALCULATOR_STACK_H
#define CALCULATOR_STACK_H

#include "Array.h"

typedef Array Stack;

int Stack_init(Stack *self);

void *Stack_top(Stack *self);

void *Stack_pop(Stack *self);

void Stack_releaseNode(void *node);

int Stack_push(Stack *self, void *p, size_t element_size);

void Stack_delete(Stack *self);


#endif //CALCULATOR_STACK_H
