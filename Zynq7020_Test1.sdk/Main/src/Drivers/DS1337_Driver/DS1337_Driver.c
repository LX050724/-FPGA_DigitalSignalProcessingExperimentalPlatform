/*
 * DS1337_Driver.c
 *
 *  Created on: 2021年12月30日
 *      Author: yaoji
 */

#include "DS1337_Driver/DS1337_Driver.h"

static const char *weekStr[] = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};

static XIicPs *default_InstancePtr = NULL;

#define INSTANCE(I) ((I) == NULL ? default_InstancePtr : (I))

/**
 * @brief 数字转Bcd码
 * 
 * @param val 
 * @return uint8_t 
 */
uint8_t DS1337_DecToBcd(uint8_t val) { return ((val / 10 * 16) + (val % 10)); }

/**
 * @brief Bcd码转数字
 * 
 * @param val 
 * @return uint8_t 
 */
uint8_t DS1337_BcdToDec(uint8_t val) { return ((val / 16 * 10) + (val % 16)); }

/**
 * @brief 从DS1337获取时间
 * 
 * @param InstancePtr IIC接口对象，传入NULL则使用默认接口
 * @param t 时间结构体
 */
void DS1337_GetTime(XIicPs *InstancePtr, struct tm *t) {
    uint8_t data[7] = {0};
    I2C_ReadReg_8BitAddr(INSTANCE(InstancePtr), DS1337_I2C_ADDRESS, 0x00, data, 7);
    t->tm_sec = DS1337_BcdToDec(data[0]);
    t->tm_min = DS1337_BcdToDec(data[1]);
    if (data[2] & 0x40) {
        t->tm_hour = DS1337_BcdToDec(data[2] & 0x1f);
        if (data[2] & 0x20) t->tm_hour += 12;
    } else {
        t->tm_hour = DS1337_BcdToDec(data[2] & 0x3f);
    }
    t->tm_wday = DS1337_BcdToDec(data[3] & 0x07);
    t->tm_mday = DS1337_BcdToDec(data[4] & 0x3f);
    t->tm_mon = DS1337_BcdToDec(data[5] & 0x1f) - 1;
    t->tm_year = DS1337_BcdToDec(data[6]) + 100;
}

/**
 * @brief 设置DS1337的时间
 * 
 * @param InstancePtr IIC接口对象，传入NULL则使用默认接口
 * @param t 时间结构体
 */
void DS1337_SetTime(XIicPs *InstancePtr, struct tm *t) {
    uint8_t data[7] = {0};
    data[0] = DS1337_DecToBcd(t->tm_sec);
    data[1] = DS1337_DecToBcd(t->tm_min);
    data[2] = DS1337_DecToBcd(t->tm_hour);
    data[3] = DS1337_DecToBcd(t->tm_wday);
    data[4] = DS1337_DecToBcd(t->tm_mday);
    data[5] = DS1337_DecToBcd(t->tm_mon + 1);
    data[6] = DS1337_DecToBcd(t->tm_year - 100);
    I2C_WriteReg_8BitAddr(INSTANCE(InstancePtr), DS1337_I2C_ADDRESS, 0x00, data, 7);
}

/**
 * @brief 返回星期字符串
 * 
 * @param week 星期数 1-7
 * @return const char* 星期字符串
 */
const char *DS1337_WeekStr(uint8_t week) {
    if (week <= 7 && week >= 1) return weekStr[week - 1];
    return "Err";
}

/**
 * @brief 设置默认IIC接口，用于get_fattime
 * 
 * @param InstancePtr 
 */
void DS1337_SetDefaultInstance(XIicPs *InstancePtr) { default_InstancePtr = InstancePtr; }

#if __has_include("ff.h")
#include "integer.h"

/**
 * @brief Get the fattime object
 * 
 * @return DWORD 
 */
DWORD get_fattime(void) {
    if (default_InstancePtr != NULL) {
        struct tm t = {0};
        DS1337_GetTime(default_InstancePtr, &t);
        return ((DWORD) (t.tm_year + 1900U - 1980U) << 25U) /* Fixed to Jan. 1, 2010 */
               | ((DWORD) (t.tm_mon + 1) << 21) | ((DWORD) t.tm_mday << 16) | ((DWORD) t.tm_hour << 11) |
               ((DWORD) t.tm_min << 5) | ((DWORD) t.tm_sec >> 1);
    } else {
        return ((DWORD) (2010U - 1980U) << 25U) /* Fixed to Jan. 1, 2010 */
               | ((DWORD) 1 << 21) | ((DWORD) 1 << 16) | ((DWORD) 0 << 11) | ((DWORD) 0 << 5) |
               ((DWORD) 0 >> 1);
    }
}

#endif
