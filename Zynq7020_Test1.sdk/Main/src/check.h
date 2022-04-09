/*
 * utils.h
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "xil_printf.h"
#include "xstatus.h"

#define CHECK_STATUS(__C)                            \
    do {                                             \
        int __STATUS = (int)__C;                     \
        if (__STATUS != XST_SUCCESS) {               \
            xil_printf(                              \
                "ERROR: File:'%s' Line:%d "          \
                "in Function '%s' return is %d\r\n", \
                __FILE__, __LINE__, #__C, __STATUS); \
        }                                            \
    } while (0)

#define CHECK_STATUS_RET(__C)                        \
    do {                                             \
        int __STATUS = (int)__C;                     \
        if (__STATUS != XST_SUCCESS) {               \
            xil_printf(                              \
                "ERROR: File:'%s' Line:%d "          \
                "in Function '%s' return is %d\r\n", \
                __FILE__, __LINE__, #__C, __STATUS); \
            return __STATUS;                         \
        }                                            \
    } while (0)

#define CHECK_STATUS_GOTO(__STATUS, __LABEL, __C)    \
    do {                                             \
        __STATUS = (int)__C;                         \
        if (__STATUS != XST_SUCCESS) {               \
            xil_printf(                              \
                "ERROR: File:'%s' Line:%d "          \
                "in Function '%s' return is %d\r\n", \
                __FILE__, __LINE__, #__C, __STATUS); \
            goto __LABEL;                            \
        }                                            \
    } while (0)

#define CHECK_FATAL_ERROR(__C)                                                              \
    if (__C) {                                                                              \
        xil_printf("FATAL ERROR: In File:'%s' Line:%d '%s'\r\n", __FILE__, __LINE__, #__C); \
        while(1);                                                                           \
    }

#endif /* SRC_UTILS_H_ */
