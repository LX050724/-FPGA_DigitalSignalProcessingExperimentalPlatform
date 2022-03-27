/*
 * FreeRTOS_Mem.c
 *
 *  Created on: 2021年12月31日
 *      Author: yaoji
 */

#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include "FreeRTOS.h"
#include "utils.h"
#include "stdlib.h"

/**
 * 开启内存统计功能
 */
#define MEM_ENABLE_STATISTICS 1

#if MEM_ENABLE_STATISTICS
typedef struct _mem_node {
    void *p;
    uint32_t size;
    struct _mem_node *last;
    struct _mem_node *next;
} mem_node;

static mem_node *head;
static mem_node *tail;

static uint64_t used_size;
static uint64_t mem_count;

static mem_node *mem_list_create_node() {
    mem_node *node = malloc(sizeof(mem_node));
    CHECK_FATAL_ERROR(node == NULL)
    memset(node, 0, sizeof(mem_node));
    return node;
}

static void mem_list_push(void *p, uint32_t size) {
    if (tail == NULL) {
        head = tail = mem_list_create_node();
        tail->p = p;
        tail->size = size;
    } else {
        mem_node *node = mem_list_create_node();
        node->p = p;
        node->size = size;
        tail->next = node;
        node->last = tail;
        tail = node;
    }
    used_size += size;
    mem_count++;
}

static mem_node *mem_list_find(void *p) {
    mem_node *node = head;
    while (node != NULL) {
        if (node->p == p)
            return node;
        node = node->next;
    }
    return NULL;
}

static void mem_list_delete(void *p) {
    mem_node *node = mem_list_find(p);
    if (node == NULL) return;
    mem_node *last = node->last;
    mem_node *next = node->next;
    if (tail == node)
        tail = last;
    if (head == node)
        head = next;
    if (last) last->next = next;
    if (next) next->last = last;
    mem_count--;
    used_size -= node->size;
    free(node);
}

static void mem_list_modify(void *old_p, void *new_p, uint32_t size) {
    if(new_p != NULL) {
        mem_node *node = mem_list_find(old_p);
        if (node != NULL) {
            used_size += size;
            used_size -= node->size;
            node->p = new_p;
            node->size = size;
        }
    }
}

uint64_t mem_get_used_size() {
    return used_size + mem_count * sizeof(mem_node);
}

uint64_t mem_get_count() {
    return mem_count;
}
#else
uint64_t mem_get_used_size() {
    return 0;
}

uint64_t mem_get_count() {
    return 0;
}
#endif

void *os_malloc(size_t __size) {
    vPortEnterCritical();
    void *p = malloc(__size);
#if MEM_ENABLE_STATISTICS
    mem_list_push(p, __size);
#endif
    vPortExitCritical();
    return p;
}

void *os_realloc(void *__r, size_t __size) {
    vPortEnterCritical();
    void *p = realloc(__r, __size);
#if MEM_ENABLE_STATISTICS
    mem_list_modify(__r, p, __size);
#endif
    vPortExitCritical();
    return p;
}

void os_free(void *__r) {
    if (__r == NULL) return;
    vPortEnterCritical();
#if MEM_ENABLE_STATISTICS
    mem_list_delete(__r);
#endif
    free(__r);
    vPortExitCritical();
}
