//
// Created by yao on 2022/2/18.
//

#include "Array.h"
#include "FreeRTOS_Mem/FreeRTOS_Mem.h"
#include <string.h>

#define CALC_MEM_CHECK(P) if((P) == NULL) { Array_delete(self); return 0;}

int Array_init(Array *self) {
    self->len = 0;
    self->p = NULL;
    return 1;
}

void Array_delete(Array *self) {
    if (self->p) {
        for (size_t i = 0; i < self->len; i++) {
            if (self->p[i]) {
                if (self->p[i]->destructor)
                    self->p[i]->destructor(self->p[i]);
                os_free(self->p[i]);
            }
        }
        os_free(self->p);
        self->p = NULL;
        self->len = 0;
    }
}

int Array_push(Array *self, void *p, size_t element_size) {
    CALC_MEM_CHECK(self->p = os_reallocarray(self->p, ++self->len, sizeof(void *)))
    CALC_MEM_CHECK(self->p[self->len - 1] = os_malloc(element_size))
    memcpy(self->p[self->len - 1], p, element_size);
    self->p[self->len - 1]->size = element_size;
    return 1;
}

void *Array_get(Array *self, size_t index) {
    if (index >= self->len) return NULL;
    else return self->p[index];
}

int Array_remove(Array *self, size_t index) {
    if (index >= self->len) return 0;
    if (self->p[index]->destructor)
        self->p[index]->destructor(self->p[index]);
    os_free(self->p[index]);
    memcpy(&self->p[index], &self->p[index + 1], (self->len - index - 1) * sizeof(void *));
    self->len--;
    return 1;
}

int Array_resize(Array *self, size_t new_len) {
    if (self->len == new_len) return 1;
    if (self->len > new_len) {
        for (size_t i = new_len; i < self->len; i++) {
            if (self->p[i]->destructor)
                self->p[i]->destructor(self->p[i]);
            os_free(self->p[i]);
        }
    }
    CALC_MEM_CHECK(self->p = os_reallocarray(self->p, new_len, sizeof(void *)))
    self->len = new_len;
    return 1;
}
