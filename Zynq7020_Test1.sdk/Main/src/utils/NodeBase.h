//
// Created by yao on 2022/2/26.
//

#ifndef TELEPHONEDIRECTORY_NODEBASE_H
#define TELEPHONEDIRECTORY_NODEBASE_H

#include <stddef.h>

/**
 * 继承该结构体以实现析构功能
 */
typedef struct {
    void (*destructor)(void *);
    size_t size;
} NodeBase;

/**
 * 泛型比较函数 大于返回1，小于返回-1，相等返回0
 */
typedef int (*CompareFunction_t)(const void *, const void *);

#endif //TELEPHONEDIRECTORY_NODEBASE_H
