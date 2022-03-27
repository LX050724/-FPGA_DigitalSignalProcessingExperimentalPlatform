/*
 * ScuGic_Driver.h
 *
 *  Created on: 2021年12月25日
 *      Author: yaoji
 */

#ifndef SRC_SCUGIC_DRIVER_SCUGIC_DRIVER_H_
#define SRC_SCUGIC_DRIVER_SCUGIC_DRIVER_H_

#include "xscugic.h"

#define INT_TYPE_RISING_EDGE 0x03
#define INT_TYPE_HIGHLEVEL 0x01

#define INT_PRIORITY_0 0
#define INT_PRIORITY_8 8
#define INT_PRIORITY_16 16
#define INT_PRIORITY_24 24
#define INT_PRIORITY_32 32
#define INT_PRIORITY_40 40
#define INT_PRIORITY_48 48
#define INT_PRIORITY_56 56
#define INT_PRIORITY_64 64
#define INT_PRIORITY_72 72
#define INT_PRIORITY_80 80
#define INT_PRIORITY_88 88
#define INT_PRIORITY_96 96
#define INT_PRIORITY_104 104
#define INT_PRIORITY_112 112
#define INT_PRIORITY_120 120
#define INT_PRIORITY_128 128
#define INT_PRIORITY_136 136
#define INT_PRIORITY_144 144
#define INT_PRIORITY_152 152
#define INT_PRIORITY_160 160
#define INT_PRIORITY_168 168
#define INT_PRIORITY_176 176
#define INT_PRIORITY_184 184
#define INT_PRIORITY_192 192
#define INT_PRIORITY_200 200
#define INT_PRIORITY_208 208
#define INT_PRIORITY_216 216
#define INT_PRIORITY_224 224
#define INT_PRIORITY_232 232
#define INT_PRIORITY_240 240
#define INT_PRIORITY_248 248

uint32_t ScuGic_GetPLIntrId(uint8_t index);

int ScuGic_Init();

int ScuGic_SetInterrupt(uint32_t Int_id, Xil_InterruptHandler Handler, void *CallBackRef,
                        uint8_t Priority, uint8_t Trigger);

extern XScuGic xInterruptController;

#endif /* SRC_SCUGIC_DRIVER_SCUGIC_DRIVER_H_ */
