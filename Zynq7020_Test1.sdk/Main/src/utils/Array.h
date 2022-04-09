//
// Created by yao on 2022/2/18.
//

#ifndef CALCULATOR_ARRAY_H
#define CALCULATOR_ARRAY_H

#include <stdint.h>
#include <stdlib.h>

#include "NodeBase.h"

/**
 * Array class
 */
typedef struct {
    NodeBase **p;
    size_t len;
} Array;


/**
 * 
 * @return 
 */
int Array_init(Array *array);

/**
 * 
 * @param array 
 */
void Array_delete(Array *array);

/**
 * 
 * @param array 
 * @param p 
 * @param element_size 
 * @return 
 */
int Array_push(Array *array, void *p, size_t element_size);

/**
 * 
 * @param array 
 * @param index 
 * @return 
 */
void *Array_get(const Array *array, size_t index);


/**
 * 
 * @param array 
 * @param index 
 * @return 
 */
int Array_remove(Array *array, size_t index);

/**
 * 
 * @param array 
 * @param new_len 
 * @return 
 */
int Array_resize(Array *array, size_t new_len);

#endif //CALCULATOR_ARRAY_H
